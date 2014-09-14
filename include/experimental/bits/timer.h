//
// timer.h
// ~~~~~~~
// Waitable timer.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_TIMER_H
#define EXECUTORS_EXPERIMENTAL_BITS_TIMER_H

#include <chrono>
#include <memory>
#include <thread>
#include <experimental/type_traits>
#include <experimental/bits/invoker.h>
#include <experimental/bits/reactor.h>
#include <experimental/bits/timer_queue.h>

namespace std {
namespace experimental {
inline namespace concurrency_v1 {

template <class _Clock, class _Duration>
typename _Clock::duration __safe_subtract(
  const chrono::time_point<_Clock, _Duration>& __t1,
  const chrono::time_point<_Clock, _Duration>& __t2)
{
  typedef chrono::time_point<_Clock, _Duration> time_point;
  typedef typename time_point::duration duration;

  time_point __epoch;
  if (__t1 >= __epoch)
  {
    if (__t2 >= __epoch)
    {
      return __t1 - __t2;
    }
    else if (__t2 == (time_point::min)())
    {
      return (duration::max)();
    }
    else if ((time_point::max)() - __t1 < __epoch - __t2)
    {
      return (duration::max)();
    }
    else
    {
      return __t1 - __t2;
    }
  }
  else // __t1 < __epoch
  {
    if (__t2 < __epoch)
    {
      return __t1 - __t2;
    }
    else if (__t1 == (time_point::min)())
    {
      return (duration::min)();
    }
    else if ((time_point::max)() - __t2 < __epoch - __t1)
    {
      return (duration::min)();
    }
    else
    {
      return -(__t2 - __t1);
    }
  }
}

template <class _Clock>
inline typename _Clock::duration timer_traits<_Clock>::to_duration(
  const typename _Clock::duration& __d)
{
  return __d;
}

template <class _Clock>
inline typename _Clock::duration timer_traits<_Clock>::to_duration(
  const typename _Clock::time_point& __t)
{
  return __safe_subtract(__t, _Clock::now());
}

template <class _Clock, class _TimerTraits>
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

private:
  template <class, class> friend class basic_timer;
  __reactor& _M_reactor;
  __timer_queue<_Clock, _TimerTraits> _M_queue;
};

template <class _Clock, class _TimerTraits>
basic_timer<_Clock, _TimerTraits>::basic_timer()
  : basic_timer(system_executor().context())
{
}

template <class _Clock, class _TimerTraits>
basic_timer<_Clock, _TimerTraits>::basic_timer(const duration& __d)
  : basic_timer(system_executor().context(), __d)
{
}

template <class _Clock, class _TimerTraits>
basic_timer<_Clock, _TimerTraits>::basic_timer(const time_point& __t)
  : basic_timer(system_executor().context(), __t)
{
}

template <class _Clock, class _TimerTraits>
basic_timer<_Clock, _TimerTraits>::basic_timer(execution_context& __c)
  : _M_service(&use_service<__timer_service<_Clock, _TimerTraits>>(__c)),
    _M_expiry((time_point::max)())
{
}

template <class _Clock, class _TimerTraits>
basic_timer<_Clock, _TimerTraits>::basic_timer(execution_context& __c, const duration& __d)
  : _M_service(&use_service<__timer_service<_Clock, _TimerTraits>>(__c)),
    _M_expiry(_Clock::now() + __d)
{
}

template <class _Clock, class _TimerTraits>
basic_timer<_Clock, _TimerTraits>::basic_timer(execution_context& __c, const time_point& __t)
  : _M_service(&use_service<__timer_service<_Clock, _TimerTraits>>(__c)),
    _M_expiry(__t)
{
}

template <class _Clock, class _TimerTraits>
basic_timer<_Clock, _TimerTraits>::basic_timer(basic_timer&& __t)
  : _M_service(__t._M_service), _M_expiry(__t._M_expiry)
{
  __t._M_expiry = (time_point::max)();
  _M_service->_M_reactor._Move_timer(_M_service->_M_queue, _M_data, __t._M_data);
}

template <class _Clock, class _TimerTraits>
basic_timer<_Clock, _TimerTraits>& basic_timer<_Clock, _TimerTraits>::operator=(basic_timer&& __t)
{
  cancel();
  _M_service = __t._M_service;
  _M_expiry = __t._M_expiry;
  __t._M_expiry = (time_point::max)();
  _M_service->_M_reactor._Move_timer(_M_service->_M_queue, _M_data, __t._M_data);
  return *this;
}

template <class _Clock, class _TimerTraits>
basic_timer<_Clock, _TimerTraits>::~basic_timer()
{
  cancel();
}

template <class _Clock, class _TimerTraits>
execution_context& basic_timer<_Clock, _TimerTraits>::context()
{
  return _M_service->context();
}

template <class _Clock, class _TimerTraits>
void basic_timer<_Clock, _TimerTraits>::cancel()
{
  _M_service->_M_reactor._Cancel_timer(_M_service->_M_queue, _M_data);
}

template <class _Clock, class _TimerTraits>
void basic_timer<_Clock, _TimerTraits>::cancel_one()
{
  _M_service->_M_reactor._Cancel_timer(_M_service->_M_queue, _M_data, 1);
}

template <class _Clock, class _TimerTraits>
typename basic_timer<_Clock, _TimerTraits>::time_point
  basic_timer<_Clock, _TimerTraits>::expiry() const
{
  return _M_expiry;
}

template <class _Clock, class _TimerTraits>
void basic_timer<_Clock, _TimerTraits>::expires_at(const time_point& __t)
{
  cancel();
  _M_expiry = __t;
}

template <class _Clock, class _TimerTraits>
void basic_timer<_Clock, _TimerTraits>::expires_after(const duration& __d)
{
  cancel();
  _M_expiry = _Clock::now() + __d;
}

template <class _Clock, class _TimerTraits>
void basic_timer<_Clock, _TimerTraits>::wait()
{
  error_code __ec;
  wait(__ec);
  if (__ec)
    throw system_error(__ec);
}

template <class _Clock, class _TimerTraits>
void basic_timer<_Clock, _TimerTraits>::wait(error_code& __ec)
{
  duration __d = traits_type::to_duration(_M_expiry);
  while (duration::zero() < __d)
  {
    this_thread::sleep_for(__d);
    __d = traits_type::to_duration(_M_expiry);
  }

  __ec.clear();
}

template <class _Clock, class _TimerTraits> template <class _CompletionToken>
auto basic_timer<_Clock, _TimerTraits>::wait(_CompletionToken&& __token)
{
  typedef handler_type_t<_CompletionToken, void(error_code)> _Handler;
  async_completion<_CompletionToken, void(error_code)> __completion(__token);

  typedef __wait_op<_Handler> _Op;
  auto __allocator(get_associated_allocator(__completion.handler));
  auto __op(_Allocate_small_block<_Op>(__allocator, std::move(__completion.handler)));

  _M_service->_M_reactor._Enqueue_timer(_M_service->_M_queue, _M_expiry, _M_data, __op.get());
  __op.release();

  return __completion.result.get();
}

} // inline namespace concurrency_v1
} // namespace experimental
} // namespace std

#endif
