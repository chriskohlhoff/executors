//
// market_data.hpp
// ~~~~~~~~~~~~~~~
// Market data events.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef MARKET_DATA_HPP
#define MARKET_DATA_HPP

#include <cstdint>
#include <iosfwd>
#include <string>
#include "order_side.hpp"

namespace market_data {

struct heartbeat
{
  std::uint64_t sequence_number;
  std::uint64_t time;
};

std::istream& operator>>(std::istream& is, heartbeat& h);
std::ostream& operator<<(std::ostream& os, const heartbeat& h);

struct new_order
{
  std::uint64_t sequence_number;
  std::string symbol;
  order_side side;
  int price;
  unsigned int quantity;
};

std::istream& operator>>(std::istream& is, new_order& o);
std::ostream& operator<<(std::ostream& os, const new_order& o);

struct trade
{
  std::uint64_t sequence_number;
  std::string symbol;
  order_side aggressor_side;
  int price;
  unsigned int quantity;
};

std::istream& operator>>(std::istream& is, trade& t);
std::ostream& operator<<(std::ostream& os, const trade& t);

} // namespace market_data

#endif
