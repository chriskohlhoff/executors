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

connection_handler::connection_handler(unsigned short port, order_management_bus& bus)
  : socket_(port), order_management_bus_(bus)
{
  std::cerr << "Listening on port " << port << std::endl;
}

void connection_handler::start()
{
  std::experimental::defer(thread_pool_.get_executor(),
      [this]
      {
        receive_and_dispatch();
        start();
      });
}

void connection_handler::join()
{
  thread_pool_.join();
}

void connection_handler::receive_and_dispatch()
{
  char buffer[1024];
  std::size_t length = socket_.receive(buffer, sizeof(buffer));
  std::istringstream is(std::string(buffer, length));
  order_management::new_order event;
  if (is >> event)
    order_management_bus_.dispatch_event(event);
}
