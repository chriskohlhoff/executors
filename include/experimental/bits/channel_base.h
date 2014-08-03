//
// channel_base.h
// ~~~~~~~~~~~~~~
// Base class for all channels.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_CHANNEL_BASE_H
#define EXECUTORS_EXPERIMENTAL_BITS_CHANNEL_BASE_H

#include <mutex>
#include <experimental/bits/operation.h>
#include <experimental/bits/small_block_recycler.h>

namespace std {
namespace experimental {
inline namespace concurrency_v1 {

class __channel_service;

class __channel_op
  : public __operation
{
public:
  void _Cancel() { _M_result = _Result::__operation_canceled; _Complete(); }
  void _Close() { _M_result = _Result::__broken_pipe; _Complete(); }
  bool _Has_value() const { return _M_has_value; }

protected:
  bool _M_has_value = false;
  enum class _Result : char
  {
    __success = 0,
    __operation_canceled = 1,
    __broken_pipe = 2
  } _M_result = _Result::__success;
};

enum class __channel_state : short
{
  buffer = 0,
  waiter = 1,
  block = 2,
  closed = 3
};

class __channel_base
{
protected:
  __channel_base(execution_context& __c, size_t __capacity);
  __channel_base(const __channel_base&) = delete;
  __channel_base(__channel_base&&);
  __channel_base& operator=(const __channel_base&) = delete;
  ~__channel_base();

  execution_context& _Context();
  void _Move_from(__channel_base&&);
  size_t _Capacity() const;
  bool _Is_open() const;
  void _Reset();
  void _Close();
  void _Cancel();
  bool _Ready() const;

  __channel_state _M_get_state = __channel_state::block;
  __channel_state _M_put_state = __channel_state::buffer;
  __op_queue<__channel_op> _M_waiters;

private:
  friend class __channel_service;
  __channel_service* _M_service = nullptr;
  size_t _M_capacity = 0;
  __channel_base* _M_next = nullptr;
  __channel_base* _M_prev = nullptr;
};

class __channel_service
  : public execution_context::service
{
public:
  __channel_service(execution_context& __c)
    : execution_context::service(__c), _M_first(nullptr)
  {
  }

  void shutdown_service()
  {
    __op_queue<__channel_op> __ops;
    lock_guard<mutex> __lock(_M_mutex);

    for (__channel_base* __i = _M_first; __i; __i = __i->_M_next)
      __ops._Push(__i->_M_waiters);
  }

private:
  friend class __channel_base;
  mutable mutex _M_mutex;
  __channel_base* _M_first;
};

inline __channel_base::__channel_base(execution_context& __c, size_t __capacity)
  : _M_service(&use_service<__channel_service>(__c)), _M_capacity(__capacity)
{
  _M_put_state = __capacity ? __channel_state::buffer : __channel_state::block;

  lock_guard<mutex> __lock(_M_service->_M_mutex);

  _M_next = _M_service->_M_first;
  if (_M_service->_M_first)
    _M_service->_M_first->_M_prev = this;
  _M_service->_M_first = this;
}

inline __channel_base::__channel_base(__channel_base&& __c)
  : _M_get_state(__c._M_get_state), _M_put_state(__c._M_put_state),
    _M_service(__c._M_service), _M_capacity(__c._M_capacity)
{
  _M_waiters._Push(__c._M_waiters);

  lock_guard<mutex> __lock(_M_service->_M_mutex);

  _M_next = _M_service->_M_first;
  if (_M_service->_M_first)
    _M_service->_M_first->_M_prev = this;
  _M_service->_M_first = this;
}

inline __channel_base::~__channel_base()
{
  lock_guard<mutex> __lock(_M_service->_M_mutex);

  if (_M_service->_M_first == this)
    _M_service->_M_first = _M_next;
  if (_M_prev)
    _M_prev->_M_next = _M_next;
  if (_M_next)
    _M_next->_M_prev = _M_prev;
}

inline execution_context& __channel_base::_Context()
{
  return _M_service->context();
}

inline void __channel_base::_Move_from(__channel_base&& __c)
{
  _Reset();

  if (_M_service != __c._M_service)
  {
    unique_lock<mutex> __lock(_M_service->_M_mutex);

    if (_M_service->_M_first == this)
      _M_service->_M_first = _M_next;
    if (_M_prev)
      _M_prev->_M_next = _M_next;
    if (_M_next)
      _M_next->_M_prev = _M_prev;
    _M_next = _M_prev = nullptr;

    _M_service = __c._M_service;
    __lock = unique_lock<mutex>(_M_service->_M_mutex);

    _M_next = _M_service->_M_first;
    if (_M_service->_M_first)
      _M_service->_M_first->_M_prev = this;
    _M_service->_M_first = this;
  }

  swap(_M_get_state, __c._M_get_state);
  swap(_M_put_state, __c._M_put_state);
  swap(_M_capacity, __c._M_capacity);
  _M_waiters._Push(__c._M_waiters);
}

inline size_t __channel_base::_Capacity() const
{
  return _M_capacity;
}

inline bool __channel_base::_Is_open() const
{
  return _M_put_state != __channel_state::closed;
}

inline void __channel_base::_Reset()
{
  _Cancel();
  if (_M_get_state == __channel_state::closed)
    _M_get_state = __channel_state::block;
  if (_M_put_state == __channel_state::closed)
    _M_put_state = _M_capacity ? __channel_state::buffer : __channel_state::block;
}

inline void __channel_base::_Close()
{
  _M_put_state = __channel_state::closed;
  switch (_M_get_state)
  {
  case __channel_state::block:
    {
      _M_get_state = __channel_state::closed;
      while (__channel_op* __op = _M_waiters._Front())
      {
        _M_waiters._Pop();
        __op->_Close();
      }
      break;
    }
  case __channel_state::buffer:
  case __channel_state::waiter:
  default:
    break;
  }
}

inline void __channel_base::_Cancel()
{
  while (__channel_op* op = _M_waiters._Front())
  {
    _M_waiters._Pop();
    op->_Cancel();
  }

  if (_M_get_state == __channel_state::waiter)
    _M_get_state = __channel_state::block;
  if (_M_put_state == __channel_state::waiter)
    _M_put_state = __channel_state::block;
}

inline bool __channel_base::_Ready() const
{
  return _M_get_state != __channel_state::block;
}

} // inline namespace concurrency_v1
} // namespace experimental
} // namespace std

#endif
