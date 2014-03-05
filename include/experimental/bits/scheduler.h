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

#include <experimental/bits/operation.h>

namespace std {
namespace experimental {

template <class _Func>
class __scheduler_op
  : public __operation
{
public:
  template <class _F> __scheduler_op(_F&& __f)
    : _M_func(forward<_F>(__f))
  {
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
};

class __scheduler
{
public:
  __scheduler() : _M_outstanding_work(0), _M_stopped(false) {} 

  template <class _F> void _Post(_F&& __f)
  {
    typedef typename decay<_F>::type _Func;
    unique_ptr<__scheduler_op<_Func>> __op(
      new __scheduler_op<_Func>(forward<_F>(__f)));

    lock_guard<mutex> lock(_M_mutex);

    _M_queue._Push(__op.get());
    if (_M_queue._Front() == __op.get())
      _M_condition.notify_one();

    __op.release();
  }

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

  size_t _Run()
  {
    unique_lock<mutex> __lock(_M_mutex);

    std::size_t __n = 0;
    for (; _Do_run_one(__lock); __lock.lock())
      if (__n != (numeric_limits<size_t>::max)())
        ++__n;
    return __n;
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

  mutex _M_mutex;
  condition_variable _M_condition;
  __op_queue<__operation> _M_queue;
  atomic<size_t> _M_outstanding_work;
  bool _M_stopped;
};

} // namespace experimental
} // namespace std

#endif
