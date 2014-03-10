//
// timed_invoker.h
// ~~~~~~~~~~~~~~~
// Function object to invoke another function object after a delay.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_TIMED_INVOKER_H
#define EXECUTORS_EXPERIMENTAL_BITS_TIMED_INVOKER_H

#include <experimental/bits/invoker.h>

namespace std {
namespace experimental {

template <class _Clock, class _Work, class _Result, class _Producer, class _Consumer>
class __timed_invoker
{
public:
  template <class _P, class _C>
  __timed_invoker(execution_context& __context,
      const typename _Clock::time_point& __abs_time,
      const _Work& __work, _P&& __producer, _C&& __consumer)
    : _M_timer(new basic_timer<_Clock>(__context, __abs_time)),
      _M_invoker{__work, forward<_P>(__producer), forward<_C>(__consumer)}
  {
  }

  template <class _P, class _C>
  __timed_invoker(execution_context& __context,
      const typename _Clock::duration& __rel_time,
      const _Work& __work, _P&& __producer, _C&& __consumer)
    : _M_timer(new basic_timer<_Clock>(__context, __rel_time)),
      _M_invoker{__work, forward<_P>(__producer), forward<_C>(__consumer)}
  {
  }

  template <class _Executor>
  void _Start(_Executor& __e)
  {
    basic_timer<_Clock>* __timer = _M_timer.get();
    __timer->async_wait(__e.wrap(std::move(*this)));
  }

  void operator()(const error_code&)
  {
    _M_invoker();
  }

private:
  unique_ptr<basic_timer<_Clock>> _M_timer;
  __invoker<_Work, _Result, _Producer, _Consumer> _M_invoker;
};

} // namespace experimental
} // namespace std

#endif
