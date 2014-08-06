//
// connect_handler.cpp
// ~~~~~~~~~~~~~~~~~~~
// Handles inbound order management transactions.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "connection_handler.hpp"
#include "common/order_management.hpp"
#include "order_management_bus.hpp"
#include <iostream>
#include <sstream>

const std::size_t thread_pool_size = 2;

connection_handler::connection_handler(unsigned short port, order_management_bus& bus)
  : socket_(port), thread_pool_(thread_pool_size), order_management_bus_(bus)
{
  std::cerr << "Listening on port " << port << std::endl;
}

void connection_handler::start()
{
  std::experimental::post(thread_pool_, [this]{ receive_and_dispatch(); });
}

void connection_handler::join()
{
  thread_pool_.join();
}

void connection_handler::receive_and_dispatch()
{
  // Wait until a new message is received.
  char buffer[1024];
  std::size_t length = socket_.receive(buffer, sizeof(buffer));

  // Wake another thread to wait for new messages.
  std::experimental::post(thread_pool_, [this]{ receive_and_dispatch(); });

  // Process the new message and pass it to the order management bus.
  std::istringstream is(std::string(buffer, length));
  order_management::new_order event;
  if (is >> event)
    order_management_bus_.dispatch_event(event);
}
