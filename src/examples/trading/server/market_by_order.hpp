//
// market_by_order.hpp
// ~~~~~~~~~~~~~~~~~~~
// Market data feed that publishes every passive order and trade.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef MARKET_BY_ORDER_HPP
#define MARKET_BY_ORDER_HPP

#include <string>
#include <experimental/executor>
#include <experimental/strand>
#include "common/udp_socket.hpp"
#include "market_data_feed.hpp"

class market_by_order : public market_data_feed
{
public:
  // Create a new market-by-order data feed sending to the specified IP address
  // and port number.
  market_by_order(const std::string& ip, unsigned short port);

  // Dispatch a market data event to the feed.
  virtual void handle_event(market_data::new_order o);
  virtual void handle_event(market_data::trade t);

private:
  // Send a heartbeat to let market data subscribers know that we're still alive.
  void send_heartbeat();

  std::experimental::strand<std::experimental::system_executor> strand_;
  udp_socket socket_;
  std::uint64_t next_sequence_number_;
};

#endif
