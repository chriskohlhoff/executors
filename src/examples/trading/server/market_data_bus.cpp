//
// market_data_bus.cpp
// ~~~~~~~~~~~~~~~~~~~
// Bus used to distribute events to all market data feeds.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "market_data_bus.hpp"

void market_data_bus::subscribe(market_data_feed& f)
{
  feeds_.push_back(&f);
}

void market_data_bus::dispatch_event(market_data::new_order o)
{
  for (auto& f: feeds_)
    f->handle_event(o);
}

void market_data_bus::dispatch_event(market_data::trade t)
{
  for (auto& f: feeds_)
    f->handle_event(t);
}
