//
// order_book.cpp
// ~~~~~~~~~~~~~~
// Base class for all order books.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "order_book.hpp"

order_book::order_book(const std::string& s)
  : symbol_(s)
{
}

order_book::~order_book()
{
}

const std::string order_book::symbol() const
{
  return symbol_;
}
