//
// market_data_dump.cpp
// ~~~~~~~~~~~~~~~~~~~~
// Client program to dump market data messages.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <iostream>
#include <sstream>
#include "common/market_data.hpp"
#include "common/udp_socket.hpp"

void handle_heartbeat(std::istream& is)
{
  market_data::heartbeat heartbeat;
  if (is >> heartbeat)
  {
    std::cout << "." << std::flush;
  }
  else
  {
    std::cout << "Invalid heartbeat message\n";
  }
}

void handle_new_order(std::istream& is)
{
  market_data::new_order new_order;
  if (is >> new_order)
  {
    std::cout << "\nNew order: ";
    std::cout << (new_order.side == order_side::buy ? "Buy " : "Sell ");
    std::cout << new_order.symbol << " ";
    std::cout << new_order.quantity << "@";
    std::cout << new_order.price << "\n";
  }
  else
  {
    std::cout << "Invalid new_order message\n";
  }
}

void handle_trade(std::istream& is)
{
  market_data::trade trade;
  if (is >> trade)
  {
    std::cout << "\nTrade: ";
    std::cout << trade.symbol << " ";
    std::cout << trade.quantity << "@";
    std::cout << trade.price << "\n";
  }
  else
  {
    std::cout << "Invalid new_order message\n";
  }
}

int main(int argc, char* argv[])
{
  if (argc != 2)
  {
    std::cerr << "Usage: market_data_dump <marketdataport>\n";
    return 1;
  }

  udp_socket sock(std::atoi(argv[1]));

  for (;;)
  {
    char buffer[1024];
    if (std::size_t length = sock.receive(buffer, sizeof(buffer)))
    {
      std::istringstream is(std::string(buffer, length));
      if (buffer[0] == 'H')
        handle_heartbeat(is);
      else if (buffer[0] == 'O')
        handle_new_order(is);
      else if (buffer[0] == 'T')
        handle_trade(is);
      else
        std::cout << "Unrecognised message type\n";
    }
  }
}
