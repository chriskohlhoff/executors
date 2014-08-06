//
// market_by_order.cpp
// ~~~~~~~~~~~~~~~~~~~
// Market data feed that publishes every passive order and trade.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "market_by_order.hpp"
#include <ctime>
#include <experimental/timer>
#include <sstream>

market_by_order::market_by_order(const std::string& ip, unsigned short port)
  : socket_(0), next_sequence_number_(0)
{
  socket_.connect(ip, port);
  std::experimental::dispatch(strand_, [this]{ send_heartbeat(); });
}

void market_by_order::handle_event(market_data::new_order o)
{
  std::experimental::dispatch(strand_,
      [=]() mutable
      {
        o.sequence_number = next_sequence_number_++;

        std::ostringstream os;
        os << o;
        std::string msg = os.str();

        socket_.send(msg.data(), msg.length());
      });
}

void market_by_order::handle_event(market_data::trade t)
{
  std::experimental::dispatch(strand_,
      [=]() mutable
      {
        t.sequence_number = next_sequence_number_++;

        std::ostringstream os;
        os << t;
        std::string msg = os.str();

        socket_.send(msg.data(), msg.length());
      });
}

void market_by_order::send_heartbeat()
{
  market_data::heartbeat h;
  h.sequence_number = next_sequence_number_;
  h.time = std::time(nullptr);

  std::ostringstream os;
  os << h;
  std::string msg = os.str();

  socket_.send(msg.data(), msg.length());

  std::experimental::defer_after(std::chrono::seconds(1),
      strand_, [this]{ send_heartbeat(); });
}
