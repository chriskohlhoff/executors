//
// timer_queue.h
// ~~~~~~~~~~~~~
// Waitable timer.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_TIMER_QUEUE_H
#define EXECUTORS_EXPERIMENTAL_BITS_TIMER_QUEUE_H

#include <chrono>
#include <memory>
#include <vector>

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

class __timer_queue_base
{
public:
  __timer_queue_base() : _M_next(nullptr) {}
  __timer_queue_base(const __timer_queue_base&) = delete;
  __timer_queue_base& operator=(const __timer_queue_base&) = delete;
  virtual ~__timer_queue_base() {}

  virtual bool _Empty() const = 0;
  virtual chrono::steady_clock::duration _Wait_duration(
    chrono::steady_clock::duration __max_duration) const = 0;
  virtual void _Get_ready_timers(__op_queue<__operation>& __ops) = 0;
  virtual void _Get_all_timers(__op_queue<__operation>& __ops) = 0;

private:
  friend class __timer_queue_set;
  __timer_queue_base* _M_next;
};

template <class _Clock>
class __timer_queue
  : public __timer_queue_base
{
public:
  typedef typename _Clock::time_point _Time_point;
  typedef typename _Clock::duration _Duration;

  class __per_timer_data
  {
  public:
    __per_timer_data() : _M_heap_index(~size_t(0)), _M_next(nullptr), _M_prev(nullptr) {}

  private:
    friend class __timer_queue;
    __op_queue<__operation> _M_ops;
    size_t _M_heap_index;
    __per_timer_data* _M_next;
    __per_timer_data* _M_prev;
  };

  __timer_queue()
    : _M_timers(), _M_heap()
  {
  }

  bool _Enqueue_timer(const _Time_point& __t, __per_timer_data& __timer, __operation* __op)
  {
    // Enqueue the timer object.
    if (__timer._M_prev == nullptr && &__timer != _M_timers)
    {
      if (__t == (_Time_point::max)())
      {
        // No heap entry is required for timers that never expire.
        __timer._M_heap_index = ~size_t(0);
      }
      else
      {
        // Put the new timer at the correct position in the heap. This is done
        // first since push_back() can throw due to allocation failure.
        __timer._M_heap_index = _M_heap.size();
        __heap_entry __e = { __t, &__timer };
        _M_heap.push_back(__e);
        _Up_heap(_M_heap.size() - 1);
      }

      // Insert the new timer into the linked list of active timers.
      __timer._M_next = _M_timers;
      __timer._M_prev = nullptr;
      if (_M_timers)
        _M_timers->_M_prev = &__timer;
      _M_timers = &__timer;
    }

    // Enqueue the individual timer operation.
    __timer._M_ops._Push(__op);

    // Interrupt reactor only if newly added timer is first to expire.
    return __timer._M_heap_index == 0 && __timer._M_ops._Front() == __op;
  }

  virtual bool _Empty() const
  {
    return _M_timers == nullptr;
  }

  virtual chrono::steady_clock::duration _Wait_duration(
    chrono::steady_clock::duration __max_duration) const
  {
    if (_M_heap.empty())
      return __max_duration;

    chrono::steady_clock::duration __d = __safe_subtract(_M_heap[0]._M_expiry, _Clock::now());
    return __d < __max_duration ? __d : __max_duration;
  }

  virtual void _Get_ready_timers(__op_queue<__operation>& __ops)
  {
    if (!_M_heap.empty())
    {
      const _Time_point __now = _Clock::now();
      while (!_M_heap.empty() && !(__now < _M_heap[0]._M_expiry))
      {
        __per_timer_data* __timer = _M_heap[0]._M_timer;
        __ops._Push(__timer->_M_ops);
        _Remove_timer(*__timer);
      }
    }
  }

  virtual void _Get_all_timers(__op_queue<__operation>& __ops)
  {
    while (_M_timers)
    {
      __per_timer_data* __timer = _M_timers;
      _M_timers = _M_timers->_M_next;
      __ops._Push(__timer->_M_ops);
      __timer->_M_next = nullptr;
      __timer->_M_prev = nullptr;
    }

    _M_heap.clear();
  }

private:
  void _Up_heap(size_t __index)
  {
    size_t __parent = (__index - 1) / 2;
    while (__index > 0 && _M_heap[__index]._M_expiry < _M_heap[__parent]._M_expiry)
    {
      _Swap_heap(__index, __parent);
      __index = __parent;
      __parent = (__index - 1) / 2;
    }
  }

  void _Down_heap(size_t __index)
  {
    size_t __child = __index * 2 + 1;
    while (__child < _M_heap.size())
    {
      size_t __min_child = (__child + 1 == _M_heap.size()
          || _M_heap[__child]._M_expiry < _M_heap[__child + 1]._M_expiry)
        ? __child : __child + 1;
      if (_M_heap[__index]._M_expiry < _M_heap[__min_child]._M_expiry)
        break;
      _Swap_heap(__index, __min_child);
      __index = __min_child;
      __child = __index * 2 + 1;
    }
  }

  void _Swap_heap(size_t __index1, size_t __index2)
  {
    __heap_entry __tmp = _M_heap[__index1];
    _M_heap[__index1] = _M_heap[__index2];
    _M_heap[__index2] = __tmp;
    _M_heap[__index1]._M_timer->_M_heap_index = __index1;
    _M_heap[__index2]._M_timer->_M_heap_index = __index2;
  }

  void _Remove_timer(__per_timer_data& __timer)
  {
    // Remove the timer from the heap.
    size_t __index = __timer._M_heap_index;
    if (!_M_heap.empty() && __index < _M_heap.size())
    {
      if (__index == _M_heap.size() - 1)
      {
        _M_heap.pop_back();
      }
      else
      {
        _Swap_heap(__index, _M_heap.size() - 1);
        _M_heap.pop_back();
        size_t __parent = (__index - 1) / 2;
        if (__index > 0 && _M_heap[__index]._M_expiry < _M_heap[__parent]._M_expiry)
          _Up_heap(__index);
        else
          _Down_heap(__index);
      }
    }

    // Remove the timer from the linked list of active timers.
    if (_M_timers == &__timer)
      _M_timers = __timer._M_next;
    if (__timer._M_prev)
      __timer._M_prev->_M_next = __timer._M_next;
    if (__timer._M_next)
      __timer._M_next->_M_prev = __timer._M_prev;
    __timer._M_next = nullptr;
    __timer._M_prev = nullptr;
  }

  __per_timer_data* _M_timers;

  struct __heap_entry
  {
    _Time_point _M_expiry;
    __per_timer_data* _M_timer;
  };

  std::vector<__heap_entry> _M_heap;
};

class __timer_queue_set
{
public:
  __timer_queue_set() : _M_first(nullptr) {}

  void _Insert(__timer_queue_base* __q)
  {
    __q->_M_next = _M_first;
    _M_first = __q;
  }

  void _Erase(__timer_queue_base* __q)
  {
    if (_M_first)
    {
      if (__q == _M_first)
      {
        _M_first = __q->_M_next;
        __q->_M_next = 0;
        return;
      }

      for (__timer_queue_base* __p = _M_first; __p->_M_next; __p = __p->_M_next)
      {
        if (__p->_M_next == __q)
        {
          __p->_M_next = __q->_M_next;
          __q->_M_next = nullptr;
          return;
        }
      }
    }
  }

  bool _All_empty() const
  {
    for (__timer_queue_base* __p = _M_first; __p; __p = __p->_M_next)
      if (!__p->_Empty())
        return false;
    return true;
  }

  chrono::steady_clock::duration _Wait_duration(
    chrono::steady_clock::duration __max_duration) const
  {
    chrono::steady_clock::duration __min_duration = __max_duration;
    for (__timer_queue_base* __p = _M_first; __p; __p = __p->_M_next)
      __min_duration = __p->_Wait_duration(__min_duration);
    return __min_duration;
  }

  void _Get_ready_timers(__op_queue<__operation>& __ops)
  {
    for (__timer_queue_base* __p = _M_first; __p; __p = __p->_M_next)
      __p->_Get_ready_timers(__ops);
  }

  void _Get_all_timers(__op_queue<__operation>& __ops)
  {
    for (__timer_queue_base* __p = _M_first; __p; __p = __p->_M_next)
      __p->_Get_all_timers(__ops);
  }

private:
  __timer_queue_base* _M_first;
};

} // inline namespace concurrency_v1
} // namespace experimental
} // namespace std

#endif
