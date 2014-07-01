//
// market_data_feed.hpp
// ~~~~~~~~~~~~~~~~~~~~
// Base class for all market data feeds.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef MARKET_DATA_FEED_HPP
#define MARKET_DATA_FEED_HPP

#include <string>
#include "common/market_data.hpp"

class market_data_feed
{
public:
  // Destroy the market data feed.
  virtual ~market_data_feed() {}

  // Dispatch a market data event to the feed.
  virtual void handle_event(market_data::new_order o) = 0;
  virtual void handle_event(market_data::trade t) = 0;
};

#endif
