//
// scheduler.h
// ~~~~~~~~~~~
// Thread-safe scheduler implementation.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_SCHEDULER_H
#define EXECUTORS_EXPERIMENTAL_BITS_SCHEDULER_H

#include <atomic>
#include <cassert>
#include <condition_variable>
#include <cstddef>
#include <limits>
#include <memory>
#include <mutex>
#include <type_traits>

#include <experimental/bits/call_stack.h>
#include <experimental/bits/operation.h>

namespace std {
namespace experimental {

class __scheduler
{
public:
  __scheduler() : _M_outstanding_work(0), _M_stopped(false) {} 

  template <class _F> void _Post(_F&& __f);
  template <class _F> void _Dispatch(_F&& __f);

  void _Work_started()
  {
    ++_M_outstanding_work;
  }

  void _Work_finished()
  {
    if (--_M_outstanding_work == 0)
      _Stop();
  }

  void _Stop()
  {
    lock_guard<mutex> __lock(_M_mutex);

    _M_stopped = true;
    _M_condition.notify_all();
  }

  bool _Stopped() const
  {
    lock_guard<mutex> __lock(_M_mutex);
    return _M_stopped;
  }

  void _Reset()
  {
    lock_guard<mutex> __lock(_M_mutex);
    _M_stopped = false;
  }

  size_t _Run()
  {
    if (_M_outstanding_work == 0)
    {
      _Stop();
      return 0;
    }

    typename __call_stack<__scheduler>::__context __c(this);

    unique_lock<mutex> __lock(_M_mutex);

    std::size_t __n = 0;
    for (; _Do_run_one(__lock); __lock.lock())
      if (__n != (numeric_limits<size_t>::max)())
        ++__n;
    return __n;
  }

  size_t _Run_one()
  {
    if (_M_outstanding_work == 0)
    {
      _Stop();
      return 0;
    }

    typename __call_stack<__scheduler>::__context __c(this);

    unique_lock<mutex> __lock(_M_mutex);
    return _Do_run_one(__lock);
  }

  size_t _Poll()
  {
    if (_M_outstanding_work == 0)
    {
      _Stop();
      return 0;
    }

    typename __call_stack<__scheduler>::__context __c(this);

    unique_lock<mutex> __lock(_M_mutex);

    std::size_t __n = 0;
    for (; _Do_poll_one(__lock); __lock.lock())
      if (__n != (numeric_limits<size_t>::max)())
        ++__n;
    return __n;
  }

  size_t _Poll_one()
  {
    if (_M_outstanding_work == 0)
    {
      _Stop();
      return 0;
    }

    typename __call_stack<__scheduler>::__context __c(this);

    unique_lock<mutex> __lock(_M_mutex);
    return _Do_poll_one(__lock);
  }

private:
  size_t _Do_run_one(unique_lock<mutex>& __lock)
  {
    while (_M_queue._Empty() && !_M_stopped)
      _M_condition.wait(__lock);

    if (_M_stopped)
      return 0;

    __operation* __op = _M_queue._Front();
    _M_queue._Pop();

    if (!_M_queue._Empty())
      _M_condition.notify_one();

    __lock.unlock();

    __op->_Complete();
    return 1;
  }

  size_t _Do_poll_one(unique_lock<mutex>& __lock)
  {
    if (_M_queue._Empty() || _M_stopped)
      return 0;

    __operation* __op = _M_queue._Front();
    _M_queue._Pop();

    if (!_M_queue._Empty())
      _M_condition.notify_one();

    __lock.unlock();

    __op->_Complete();
    return 1;
  }

  mutable mutex _M_mutex;
  condition_variable _M_condition;
  __op_queue<__operation> _M_queue;
  atomic<size_t> _M_outstanding_work;
  bool _M_stopped;
};

template <class _Func>
class __scheduler_op
  : public __operation
{
public:
  template <class _F> __scheduler_op(_F&& __f, __scheduler& __s)
    : _M_func(forward<_F>(__f)), _M_owner(__s)
  {
  }

  ~__scheduler_op()
  {
    _M_owner._Work_finished();
  }

  virtual void _Complete()
  {
    unique_ptr<__scheduler_op> __op(this);
    _M_func();
  }

  virtual void _Destroy()
  {
    delete this;
  }

private:
  _Func _M_func;
  __scheduler& _M_owner;
};

template <class _F> void __scheduler::_Post(_F&& __f)
{
  typedef typename decay<_F>::type _Func;
  unique_ptr<__scheduler_op<_Func>> __op(
    new __scheduler_op<_Func>(forward<_F>(__f), *this));

  lock_guard<mutex> lock(_M_mutex);

  _M_queue._Push(__op.get());
  _Work_started();

  if (_M_queue._Front() == __op.get())
    _M_condition.notify_one();

  __op.release();
}

template <class _F> void __scheduler::_Dispatch(_F&& __f)
{
  typedef typename decay<_F>::type _Func;
  if (__call_stack<__scheduler>::_Contains(this))
  {
    _Func __tmp(forward<_F>(__f));
    __tmp();
  }
  else
  {
    this->_Post(forward<_F>(__f));
  }
}

} // namespace experimental
} // namespace std

#endif
