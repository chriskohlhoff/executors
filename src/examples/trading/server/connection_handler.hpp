//
// connect_handler.hpp
// ~~~~~~~~~~~~~~~~~~~
// Handles inbound order management transactions.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef CONNECTION_HANDLER_HPP
#define CONNECTION_HANDLER_HPP

#include <experimental/thread_pool>
#include "common/udp_socket.hpp"

class order_management_bus;

class connection_handler
{
public:
  // Create a connection handler to listen on the specified port.
  connection_handler(unsigned short port, order_management_bus& bus);

  // Run the connection handler to listen for inbound messages. Any valid order
  // messages that are received are passed to the order management bus.
  void start();

  // Wait for connection handler to exit.
  void join();

private:
  // Helper function to receive and dispatch a single message.
  void receive_and_dispatch();

  udp_socket socket_;
  std::experimental::thread_pool thread_pool_;
  order_management_bus& order_management_bus_;
};

#endif
