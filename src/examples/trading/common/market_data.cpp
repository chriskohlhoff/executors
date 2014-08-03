//
// market_data.cpp
// ~~~~~~~~~~~~~~~
// Market data events.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "market_data.hpp"
#include <istream>
#include <ostream>

namespace market_data {

std::istream& operator>>(std::istream& is, heartbeat& h)
{
  if (is.get() == 'H')
    is >> h.sequence_number >> h.time;
  else
    is.setstate(std::ios::badbit);
  return is;
}

std::ostream& operator<<(std::ostream& os, const heartbeat& h)
{
  os << 'H' << ' ' << h.sequence_number << ' ' << h.time;
  return os;
}

std::istream& operator>>(std::istream& is, new_order& o)
{
  if (is.get() == 'O')
    is >> o.sequence_number >> o.symbol >> o.side >> o.price >> o.quantity;
  else
    is.setstate(std::ios::badbit);
  return is;
}

std::ostream& operator<<(std::ostream& os, const new_order& o)
{
  os << 'O' << ' ' << o.sequence_number << ' ' << o.symbol << ' ' << o.side << ' ' << o.price << ' ' << o.quantity;
  return os;
}

std::istream& operator>>(std::istream& is, trade& t)
{
  if (is.get() == 'T')
    is >> t.sequence_number >> t.symbol >> t.aggressor_side >> t.price >> t.quantity;
  else
    is.setstate(std::ios::badbit);
  return is;
}

std::ostream& operator<<(std::ostream& os, const trade& t)
{
  os << 'T' << ' ' << t.sequence_number << ' ' << t.symbol << ' ' << t.aggressor_side << ' ' << t.price << ' ' << t.quantity;
  return os;
}

} // namespace market_data
