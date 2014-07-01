//
// order_book.hpp
// ~~~~~~~~~~~~~~
// Base class for all order books.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ORDER_BOOK_HPP
#define ORDER_BOOK_HPP

#include <string>
#include "common/order_management.hpp"

class order_book
{
public:
  // Create the order book with the specified symbol.
  order_book(const std::string& s);

  // Destroy the order book.
  virtual ~order_book();

  // Get the order book's symbol.
  const std::string symbol() const;

  // Dispatch an order management event to the order book.
  virtual void handle_event(order_management::new_order o) = 0;

private:
  const std::string symbol_;
};

#endif
