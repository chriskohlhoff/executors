//
// order_side.cpp
// ~~~~~~~~~~~~~~
// Enumeration used to indicate whether an order is a buy or a sell.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "order_side.hpp"
#include <istream>
#include <ostream>

std::istream& operator>>(std::istream& is, order_side& s)
{
  char side = 0;
  is >> side;
  switch (side)
  {
  case 'B':
    s = order_side::buy;
    break;
  case 'S':
    s = order_side::sell;
    break;
  default:
    is.setstate(std::ios::badbit);
    break;
  }
  return is;
}

std::ostream& operator<<(std::ostream& os, const order_side& s)
{
  os << static_cast<char>(s);
  return os;
}
