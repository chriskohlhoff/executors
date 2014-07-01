//
// enter_one_order.cpp
// ~~~~~~~~~~~~~~~~~~~
// Program to enter a single order.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <iostream>
#include <sstream>
#include "common/order_management.hpp"
#include "common/udp_socket.hpp"

int main(int argc, char* argv[])
{
  if (argc != 8)
  {
    std::cerr << "Usage: enter_one_order <serverip> <serverport> <symbol> <side> <price> <quantity> <ioc>\n";
    return 1;
  }

  udp_socket sock(0);
  sock.connect(argv[1], std::atoi(argv[2]));

  // Create the message.
  order_management::new_order new_order;
  new_order.symbol = argv[3];
  new_order.side = (argv[4][0] == 'B') ? order_side::buy : order_side::sell;
  new_order.price = std::atoi(argv[5]);
  new_order.quantity = std::atoi(argv[6]);
  new_order.immediate_or_cancel = (std::atoi(argv[7]) != 0);

  // Encode and send the message.
  std::ostringstream os;
  os << new_order;
  std::string msg = os.str();
  sock.send(msg.data(), msg.length());
}
