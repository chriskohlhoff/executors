//
// reactor.h
// ~~~~~~~~~
// The reactor.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_REACTOR_H
#define EXECUTORS_EXPERIMENTAL_BITS_REACTOR_H

#include <condition_variable>
#include <mutex>
#include <thread>
#include <experimental/executor>
#include <experimental/bits/timer_queue.h>

namespace std {
namespace experimental {
inline namespace concurrency_v1 {

class __reactor
  : public execution_context::service
{
public:
  __reactor(execution_context& __c)
    : execution_context::service(__c),
      _M_stopped(false)
  {
    _M_thread = std::thread([this](){ _Run(); });
  }

  ~__reactor()
  {
    unique_lock<mutex> __lock(_M_mutex);
    _M_stopped = true;
    _M_condition.notify_one();
    __lock.unlock();

    _M_thread.join();
  }

  void shutdown_service()
  {
    unique_lock<mutex> __lock(_M_mutex);
    __op_queue<__operation> __ops;
    _M_queues._Get_all_timers(__ops);
    __ops._Push(_M_ops);
    __lock.unlock();
  }

  template <class _Clock, class _TimerTraits>
  void _Add_timer_queue(__timer_queue<_Clock, _TimerTraits>& __q)
  {
    unique_lock<mutex> __lock(_M_mutex);
    _M_queues._Insert(&__q);
  }

  template <class _Clock, class _TimerTraits>
  void _Remove_timer_queue(__timer_queue<_Clock, _TimerTraits>& __q)
  {
    unique_lock<mutex> __lock(_M_mutex);
    _M_queues._Erase(&__q);
  }

  template <class _Clock, class _TimerTraits>
  void _Move_timer(__timer_queue<_Clock, _TimerTraits>& __q,
    typename __timer_queue<_Clock, _TimerTraits>::__per_timer_data& __empty_target,
    typename __timer_queue<_Clock, _TimerTraits>::__per_timer_data& __source)
  {
    unique_lock<mutex> __lock(_M_mutex);
    __q._Move_timer(__empty_target, __source);
  }

  template <class _Clock, class _TimerTraits>
  void _Enqueue_timer(__timer_queue<_Clock, _TimerTraits>& __q,
    const typename _Clock::time_point& __expiry,
    typename __timer_queue<_Clock, _TimerTraits>::__per_timer_data& __timer,
    __wait_op_base* __op)
  {
    unique_lock<mutex> __lock(_M_mutex);
    if (__q._Enqueue_timer(__expiry, __timer, __op))
      _M_condition.notify_one();
  }

  template <class _Clock, class _TimerTraits>
  void _Cancel_timer(__timer_queue<_Clock, _TimerTraits>& __q,
      typename __timer_queue<_Clock, _TimerTraits>::__per_timer_data& __timer,
      size_t __max = ~size_t(0))
  {
    unique_lock<mutex> __lock(_M_mutex);
    if (__q._Cancel_timer(__timer, _M_ops, __max))
      _M_condition.notify_one();
  }

private:
  void _Run()
  {
    unique_lock<mutex> __lock(_M_mutex);
    while (!_M_stopped)
    {
      _M_queues._Get_ready_timers(_M_ops);
      for (; !_M_ops._Empty(); __lock.lock())
      {
        __operation* __op = _M_ops._Front();
        _M_ops._Pop();
        __lock.unlock();
        __op->_Complete();
      }

      _M_condition.wait_for(__lock, _M_queues._Wait_duration(chrono::seconds(60)));
    }
  }

  thread _M_thread;
  mutable mutex _M_mutex;
  condition_variable _M_condition;
  __timer_queue_set _M_queues;
  __op_queue<__operation> _M_ops;
  bool _M_stopped;
};

} // inline namespace concurrency_v1
} // namespace experimental
} // namespace std

#endif
