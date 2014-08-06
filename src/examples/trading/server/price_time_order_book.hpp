//
// price_time_order_book.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
// Order book using price-time priority.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef PRICE_TIME_ORDER_BOOK_HPP
#define PRICE_TIME_ORDER_BOOK_HPP

#include <chrono>
#include <cstdint>
#include <queue>
#include <string>
#include <experimental/executor>
#include <experimental/strand>
#include "order_book.hpp"

class market_data_bus;

class price_time_order_book : public order_book
{
public:
  // Create the order book with the specified symbol.
  price_time_order_book(const std::string& s, market_data_bus& bus);

  // Dispatch an order management event to the order book.
  virtual void handle_event(order_management::new_order o);

protected:
  // Process a new order and find possible matches.
  void process_new_order(order_management::new_order o);

  // Finds at most one match. Returns true if matched, false otherwise.
  bool find_match(order_management::new_order& o);

  // Add the order to the book.
  void add_order(const order_management::new_order& o);

  struct order
  {
    int price;
    std::uint64_t time;
    unsigned int quantity;
    bool operator<(const order& other) const;
  };

  std::experimental::strand<std::experimental::system_executor> strand_;
  market_data_bus& market_data_bus_;
  std::uint64_t next_time_;
  std::priority_queue<order> buy_orders_;
  std::priority_queue<order> sell_orders_;
};

#endif
