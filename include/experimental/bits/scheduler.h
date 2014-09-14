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
#include <experimental/bits/small_block_recycler.h>

namespace std {
namespace experimental {
inline namespace concurrency_v1 {

class __scheduler
{
public:
  struct _Context
  {
    __scheduler* _M_scheduler;
    __op_queue<__operation> _M_private_queue;
    __call_stack<__scheduler, _Context>::__context _M_context;
    unique_lock<mutex> _M_lock;
    ptrdiff_t _M_work_delta;

    explicit _Context(__scheduler* __s)
      : _M_scheduler(__s), _M_context(__s, *this), _M_lock(__s->_M_mutex),
        _M_work_delta(0)
    {
    }

    ~_Context()
    {
      if (_M_work_delta)
        if (_M_scheduler->_M_outstanding_work.fetch_add(_M_work_delta) == -_M_work_delta)
          _M_scheduler->_Stop();

      if (!_M_private_queue._Empty())
      {
        if (!_M_lock.owns_lock())
          _M_lock.lock();

        _M_scheduler->_M_queue._Push(_M_private_queue);
      }
    }

    void _Lock()
    {
      if (_M_work_delta)
      {
        if (_M_scheduler->_M_outstanding_work.fetch_add(_M_work_delta) == -_M_work_delta)
          _M_scheduler->_Stop();
        _M_work_delta = 0;
      }

      if (!_M_lock.owns_lock())
        _M_lock.lock();

      if (!_M_private_queue._Empty())
        _M_scheduler->_M_queue._Push(_M_private_queue);
    }
  };

  typedef __call_stack<__scheduler, _Context> _Call_stack;

  __scheduler(size_t __concurrency_hint = ~size_t(0))
    : _M_outstanding_work(0), _M_stopped(false),
      _M_one_thread(__concurrency_hint == 1)
  {
  }

  bool _Running_in_this_thread() const noexcept
  {
    return _Call_stack::_Contains(const_cast<__scheduler*>(this)) != nullptr;
  }

  template <class _F, class _A> void _Dispatch(_F&& __f, const _A& __a);
  template <class _F, class _A> void _Post(_F&& __f, const _A& __a);
  template <class _F, class _A> void _Defer(_F&& __f, const _A& __a);

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

  void _Restart()
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

    _Context __ctx(this);

    std::size_t __n = 0;
    for (; _Do_run_one(__ctx); __ctx._Lock())
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

    _Context __ctx(this);

    return _Do_run_one(__ctx);
  }

  template <class _Rep, class _Period>
  size_t _Run_for(const chrono::duration<_Rep, _Period>& __rel_time)
  {
    return this->_Run_until(chrono::steady_clock::now() + __rel_time);
  }

  template <class _Rep, class _Period>
  size_t _Run_one_for(const chrono::duration<_Rep, _Period>& __rel_time)
  {
    return this->_Run_one_until(chrono::steady_clock::now() + __rel_time);
  }

  template <class _Clock, class _Duration>
  size_t _Run_until(const chrono::time_point<_Clock, _Duration>& __abs_time)
  {
    if (_M_outstanding_work == 0)
    {
      _Stop();
      return 0;
    }

    _Context __ctx(this);

    std::size_t __n = 0;
    for (; _Do_run_one_until(__ctx, __abs_time); __ctx._Lock())
      if (__n != (numeric_limits<size_t>::max)())
        ++__n;
    return __n;
  }

  template <class _Clock, class _Duration>
  size_t _Run_one_until(const chrono::time_point<_Clock, _Duration>& __abs_time)
  {
    if (_M_outstanding_work == 0)
    {
      _Stop();
      return 0;
    }

    _Context __ctx(this);

    return _Do_run_one_until(__ctx, __abs_time);
  }

  size_t _Poll()
  {
    if (_M_outstanding_work == 0)
    {
      _Stop();
      return 0;
    }

    _Context __ctx(this);

    std::size_t __n = 0;
    for (; _Do_poll_one(__ctx); __ctx._Lock())
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

    _Context __ctx(this);

    return _Do_poll_one(__ctx);
  }

private:
  size_t _Do_run_one(_Context& __ctx)
  {
    while (_M_queue._Empty() && !_M_stopped)
      _M_condition.wait(__ctx._M_lock);

    if (_M_stopped)
      return 0;

    __operation* __op = _M_queue._Front();
    _M_queue._Pop();

    if (!_M_one_thread && !_M_queue._Empty())
      _M_condition.notify_one();

    __ctx._M_lock.unlock();
    __ctx._M_work_delta = -1;

    __op->_Complete();
    return 1;
  }

  template <class _Clock, class _Duration>
  size_t _Do_run_one_until(_Context& __ctx,
    const chrono::time_point<_Clock, _Duration>& __abs_time)
  {
    if (_Clock::now() >= __abs_time)
      return 0;

    while (_M_queue._Empty() && !_M_stopped)
      if (_M_condition.wait_until(__ctx._M_lock, __abs_time) == cv_status::timeout)
        return 0;

    if (_M_stopped)
      return 0;

    __operation* __op = _M_queue._Front();
    _M_queue._Pop();

    if (!_M_one_thread && !_M_queue._Empty())
      _M_condition.notify_one();

    __ctx._M_lock.unlock();
    __ctx._M_work_delta = -1;

    __op->_Complete();
    return 1;
  }

  size_t _Do_poll_one(_Context& __ctx)
  {
    if (_M_queue._Empty() || _M_stopped)
      return 0;

    __operation* __op = _M_queue._Front();
    _M_queue._Pop();

    if (!_M_one_thread && !_M_queue._Empty())
      _M_condition.notify_one();

    __ctx._M_lock.unlock();
    __ctx._M_work_delta = -1;

    __op->_Complete();
    return 1;
  }

  mutable mutex _M_mutex;
  condition_variable _M_condition;
  __op_queue<__operation> _M_queue;
  atomic<ptrdiff_t> _M_outstanding_work;
  bool _M_stopped;
  const bool _M_one_thread;
};

template <class _Func, class _Allocator>
class __scheduler_op
  : public __operation
{
public:
  __scheduler_op(const __scheduler_op&) = delete;
  __scheduler_op& operator=(const __scheduler_op&) = delete;

  template <class _F> __scheduler_op(_F&& __f, const _Allocator& __a)
    : _M_func(forward<_F>(__f)), _M_allocator(__a)
  {
  }

  __scheduler_op(__scheduler_op&& __s)
    : _M_func(std::move(__s._M_func)), _M_allocator(std::move(__s._M_allocator))
  {
  }

  ~__scheduler_op()
  {
  }

  virtual void _Complete()
  {
    auto __op(_Adopt_small_block(_M_allocator, this));
    _Func __tmp(std::move(_M_func));
    __op.reset();
    std::move(__tmp)();
  }

  virtual void _Destroy()
  {
    _Adopt_small_block(_M_allocator, this);
  }

private:
  _Func _M_func;
  _Allocator _M_allocator;
};

template <class _F, class _A> void __scheduler::_Dispatch(_F&& __f, const _A& __a)
{
  typedef typename decay<_F>::type _Func;
  if (_Call_stack::_Contains(this))
  {
    _Func(forward<_F>(__f))();
  }
  else
  {
    this->_Post(forward<_F>(__f), __a);
  }
}

template <class _F, class _A> void __scheduler::_Post(_F&& __f, const _A& __a)
{
  typedef typename decay<_F>::type _Func;
  auto __op(_Allocate_small_block<__scheduler_op<_Func, _A>>(__a, forward<_F>(__f), __a));

  _Context* __ctx = _Call_stack::_Contains(this);
  if (__ctx == nullptr)
  {
    ++_M_outstanding_work;
  }
  else if (_M_one_thread)
  {
    ++__ctx->_M_work_delta;
    __ctx->_M_private_queue._Push(__op.get());

    __op.release();
    return;
  }
  else if (__ctx->_M_work_delta < 0)
  {
    ++__ctx->_M_work_delta;
  }
  else
  {
    ++_M_outstanding_work;
  }

  lock_guard<mutex> lock(_M_mutex);

  _M_queue._Push(__op.get());
  if (_M_queue._Front() == __op.get())
    _M_condition.notify_one();

  __op.release();
}

template <class _F, class _A> void __scheduler::_Defer(_F&& __f, const _A& __a)
{
  typedef typename decay<_F>::type _Func;
  auto __op(_Allocate_small_block<__scheduler_op<_Func, _A>>(__a, forward<_F>(__f), __a));

  _Context* __ctx = _Call_stack::_Contains(this);
  if (__ctx == nullptr)
  {
    ++_M_outstanding_work;
  }
  else
  {
    ++__ctx->_M_work_delta;
    __ctx->_M_private_queue._Push(__op.get());

    __op.release();
    return;
  }

  lock_guard<mutex> lock(_M_mutex);

  _M_queue._Push(__op.get());
  if (_M_queue._Front() == __op.get())
    _M_condition.notify_one();

  __op.release();
}

} // inline namespace concurrency_v1
} // namespace experimental
} // namespace std

#endif
