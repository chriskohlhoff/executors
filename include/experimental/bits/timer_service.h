//
// timer_service.h
// ~~~~~~~~~~~~~~~
// Manages a timer queue as a service.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_TIMER_SERVICE_H
#define EXECUTORS_EXPERIMENTAL_BITS_TIMER_SERVICE_H

#include <experimental/bits/reactor.h>
#include <experimental/bits/timer_queue.h>

namespace std {
namespace experimental {
inline namespace concurrency_v1 {

template <class _Clock>
class __timer_service
  : public execution_context::service
{
public:
  __timer_service(execution_context& __c)
    : execution_context::service(__c),
      _M_reactor(use_service<__reactor>(__c))
  {
    _M_reactor._Add_timer_queue(_M_queue);
  }

  ~__timer_service()
  {
    _M_reactor._Remove_timer_queue(_M_queue);
  }

  void shutdown_service()
  {
  }

  __reactor& _M_reactor;
  __timer_queue<_Clock> _M_queue;
};

} // inline namespace concurrency_v1
} // namespace experimental
} // namespace std

#endif
