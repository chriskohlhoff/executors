//
// order_management_bus.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~
// Bus used to distribute order management events to all order books.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "order_management_bus.hpp"

void order_management_bus::subscribe(order_book& b)
{
  books_.insert(std::make_pair(b.symbol(), &b));
}

void order_management_bus::dispatch_event(order_management::new_order o)
{
  auto iter = books_.find(o.symbol);
  if (iter != books_.end())
    iter->second->handle_event(o);
}
