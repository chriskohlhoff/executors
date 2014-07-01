//
// market_data_bus.hpp
// ~~~~~~~~~~~~~~~~~~~
// Bus used to distribute events to all market data feeds.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef MARKET_DATA_BUS_HPP
#define MARKET_DATA_BUS_HPP

#include <vector>
#include "common/market_data.hpp"
#include "market_data_feed.hpp"

class market_data_bus
{
public:
  // Subscribe a market data feed to the bus.
  void subscribe(market_data_feed& f);

  // Dispatch a market data event to the bus.
  void dispatch_event(market_data::new_order o);
  void dispatch_event(market_data::trade t);

private:
  std::vector<market_data_feed*> feeds_;
};

#endif
