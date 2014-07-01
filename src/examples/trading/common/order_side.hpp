//
// order_side.hpp
// ~~~~~~~~~~~~~~
// Enumeration used to indicate whether an order is a buy or a sell.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ORDER_SIDE_HPP
#define ORDER_SIDE_HPP

#include <iosfwd>

enum class order_side
{
  buy = 'B',
  sell = 'S'
};

std::istream& operator>>(std::istream& is, order_side& s);
std::ostream& operator<<(std::ostream& os, const order_side& s);

#endif
