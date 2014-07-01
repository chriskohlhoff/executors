//
// order_management_bus.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~
// Bus used to distribute order management events to all order books.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ORDER_MANAGEMENT_BUS_HPP
#define ORDER_MANAGEMENT_BUS_HPP

#include <string>
#include <unordered_map>
#include "common/order_management.hpp"
#include "order_book.hpp"

class order_management_bus
{
public:
  // Subscribe an order book to the bus.
  void subscribe(order_book& b);

  // Dispatch an order management event to the bus.
  void dispatch_event(order_management::new_order o);

private:
  std::unordered_map<std::string, order_book*> books_;
};

#endif
