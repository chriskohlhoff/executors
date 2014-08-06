//
// order_management.cpp
// ~~~~~~~~~~~~~~~~~~~~
// Order mangement events.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "order_management.hpp"
#include <istream>
#include <ostream>

namespace order_management {

std::istream& operator>>(std::istream& is, new_order& o)
{
  if (is.get() == 'S')
    is >> o.symbol >> o.side >> o.price >> o.quantity >> o.immediate_or_cancel;
  else
    is.setstate(std::ios::badbit);
  return is;
}

std::ostream& operator<<(std::ostream& os, const new_order& o)
{
  os << 'S' << ' ' << o.symbol << ' ' << o.side << ' ' << o.price << ' ' << o.quantity << ' ' << o.immediate_or_cancel;
  return os;
}

} // namespace order_management
