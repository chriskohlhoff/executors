//
// price_time_order_book.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
// Order book using price-time priority.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "price_time_order_book.hpp"
#include <iostream>
#include <tuple>
#include "market_data_bus.hpp"

price_time_order_book::price_time_order_book(const std::string& s, market_data_bus& bus)
  : order_book(s), market_data_bus_(bus), next_time_(0)
{
  std::cerr << "Maintaining a price-time priority order book for " << symbol() << std::endl;
}

void price_time_order_book::handle_event(order_management::new_order o)
{
  std::experimental::dispatch(strand_, [=]{ process_new_order(o); });
}

void price_time_order_book::process_new_order(order_management::new_order o)
{
  // Generate all possible matches.
  while (o.quantity > 0)
    if (!find_match(o))
      break;

  // Enter any remaining quantity into the book.
  if (o.quantity > 0 && !o.immediate_or_cancel)
    add_order(o);
}

bool price_time_order_book::find_match(order_management::new_order& o)
{
  auto& other_side = (o.side == order_side::buy) ? sell_orders_ : buy_orders_;

  // Is there anything at all to match on the other side?
  if (other_side.empty())
    return false;

  // Do we cross the order on the other side of the book?
  order passive_order = other_side.top();
  if ((o.side == order_side::buy && o.price < passive_order.price)
      || (o.side == order_side::sell && o.price > passive_order.price))
    return false;

  // Work out how much is going to trade and deduct from both orders.
  unsigned int trade_quantity = std::min(o.quantity, passive_order.quantity);
  o.quantity -= trade_quantity;
  passive_order.quantity -= trade_quantity;

  // If the passive order has no remaining quantity, permanently remove it from
  // the book. Otherwise, reenter it into the book with its updated quantity.
  other_side.pop();
  if (passive_order.quantity > 0)
    other_side.push(passive_order);

  // Dispatch the trade to the market data bus.
  market_data::trade md_trade;
  md_trade.sequence_number = 0;
  md_trade.symbol = symbol();
  md_trade.aggressor_side = o.side;
  md_trade.price = passive_order.price;
  md_trade.quantity = trade_quantity;
  market_data_bus_.dispatch_event(md_trade);

  return true;
}

void price_time_order_book::add_order(const order_management::new_order& o)
{
  // Add the order to the book.
  order passive_order;
  passive_order.price = o.price;
  passive_order.time = next_time_++;
  passive_order.quantity = o.quantity;
  auto& this_side = (o.side == order_side::buy) ? buy_orders_ : sell_orders_;
  this_side.push(passive_order);

  // Dispatch the new order to the market data bus.
  market_data::new_order md_new_order;
  md_new_order.sequence_number = 0;
  md_new_order.symbol = symbol();
  md_new_order.side = o.side;
  md_new_order.price = o.price;
  md_new_order.quantity = o.quantity;
  market_data_bus_.dispatch_event(md_new_order);
}

bool price_time_order_book::order::operator<(const order& other) const
{
  return std::tie(price, time) < std::tie(other.price, other.time);
}
