//
// server.cpp
// ~~~~~~~~~~
// Entry point for the server.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <list>
#include "connection_handler.hpp"
#include "market_data_bus.hpp"
#include "market_by_order.hpp"
#include "order_management_bus.hpp"
#include "price_time_order_book.hpp"

int main(int argc, char* argv[])
{
  if (argc != 5)
  {
    std::cerr << "Usage: server <marketbyorderip> <marketbyorderport> <symbols> <ports>\n";
    return 1;
  }

  // Create the market-by-order data feed.
  market_by_order by_order(argv[1], std::atoi(argv[2]));

  // Create the market data bus and subscribe the data feed to it.
  market_data_bus md_bus;
  md_bus.subscribe(by_order);

  // Read in list of symbols and create all order books.
  std::ifstream symbol_file(argv[3]);
  std::istream_iterator<std::string> symbol_iter(symbol_file), symbol_end;
  std::list<price_time_order_book> books;
  for (; symbol_iter != symbol_end; ++symbol_iter)
    books.emplace_back(*symbol_iter, md_bus);
  symbol_file.close();

  // Create the order management bus and subscribe all books to it.
  order_management_bus om_bus;
  for (auto& book : books)
    om_bus.subscribe(book);

  // Read in the list of ports and create all connection handlers.
  std::ifstream ports_file(argv[4]);
  std::istream_iterator<unsigned short> ports_iter(ports_file), ports_end;
  std::list<connection_handler> connections;
  for (; ports_iter != ports_end; ++ports_iter)
    connections.emplace_back(*ports_iter, om_bus);
  ports_file.close();

  // Start all connection handlers.
  for (auto& connection : connections)
    connection.start();

  // Wait for all connection handlers to exit, i.e. never.
  for (auto& connection : connections)
    connection.join();
}
