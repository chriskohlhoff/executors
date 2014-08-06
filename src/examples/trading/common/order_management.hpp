//
// order_management.hpp
// ~~~~~~~~~~~~~~~~~~~~
// Order mangement events.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ORDER_MANAGEMENT_HPP
#define ORDER_MANAGEMENT_HPP

#include <iosfwd>
#include <string>
#include "order_side.hpp"

namespace order_management {

struct new_order
{
  std::string symbol;
  order_side side;
  int price;
  unsigned int quantity;
  bool immediate_or_cancel;
};

std::istream& operator>>(std::istream& is, new_order& s);
std::ostream& operator<<(std::ostream& os, const new_order& s);

} // namespace order_management

#endif
