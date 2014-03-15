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

namespace std {
namespace experimental {

template <class _Clock, class _Handler>
class __timed_invoker
{
public:
  template <class _H>
  __timed_invoker(execution_context& __context,
      const typename _Clock::time_point& __abs_time, _H&& __h)
    : _M_timer(new basic_timer<_Clock>(__context, __abs_time)),
      _M_handler(forward<_H>(__h)),
      _M_handler_work(get_executor(_M_handler).make_work())
  {
  }

  template <class _H>
  __timed_invoker(execution_context& __context,
      const typename _Clock::duration& __rel_time, _H&& __h)
    : _M_timer(new basic_timer<_Clock>(__context, __rel_time)),
      _M_handler(forward<_H>(__h)),
      _M_handler_work(get_executor(_M_handler).make_work())
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
    _M_handler();
  }

private:
  unique_ptr<basic_timer<_Clock>> _M_timer;
  _Handler _M_handler;
  typename decltype(get_executor(declval<_Handler>()))::work _M_handler_work;
};

} // namespace experimental
} // namespace std

#endif
