//
// channel_void.h
// ~~~~~~~~~~~~~~
// A channel of void "values".
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_CHANNEL_VOID_H
#define EXECUTORS_EXPERIMENTAL_BITS_CHANNEL_VOID_H

#include <experimental/executor>
#include <experimental/memory>
#include <experimental/bits/tuple_utils.h>

namespace std {
namespace experimental {
inline namespace concurrency_v1 {

template <class _Cont> template <class _Handler>
class channel<void, _Cont>::_Op
  : public __channel_op
{
public:
  typedef associated_executor_t<_Handler> _Executor;
  typedef executor_work<_Executor> _Work;

  template <class _H> explicit _Op(_H&& __h)
    : _M_handler(forward<_H>(__h)),
      _M_work(get_associated_executor(_M_handler))
  {
  }

  virtual void _Complete()
  {
    auto __allocator(get_associated_allocator(_M_handler));
    auto __op(_Adopt_small_block(__allocator, this));

    error_code __ec;
    if (this->_M_result == __channel_op::_Result::__operation_canceled)
      __ec = make_error_code(errc::operation_canceled);
    else if (this->_M_result == __channel_op::_Result::__broken_pipe)
      __ec = make_error_code(errc::broken_pipe);

    executor_work<_Executor> __work(std::move(_M_work));
    _Executor __executor(__work.get_executor());
    auto __i(_Make_tuple_invoker(std::move(_M_handler), __ec));
    __op.reset();
    __executor.post(std::move(__i), __allocator);
  }

  virtual void _Destroy()
  {
    auto __allocator(get_associated_allocator(_M_handler));
    _Adopt_small_block(__allocator, this);
  }

private:
  _Handler _M_handler;
  _Work _M_work;
};

template <class _Cont>
inline channel<void, _Cont>::channel()
  : __channel_base(system_executor().context(), 0), _M_buffered(0)
{
}

template <class _Cont>
inline channel<void, _Cont>::channel(size_t __capacity)
  : __channel_base(system_executor().context(), __capacity), _M_buffered(0)
{
}

template <class _Cont>
inline channel<void, _Cont>::channel(execution_context& __c)
  : __channel_base(__c, 0), _M_buffered(0)
{
}

template <class _Cont>
inline channel<void, _Cont>::channel(execution_context& __c, size_t __capacity)
  : __channel_base(__c, __capacity), _M_buffered(0)
{
}

template <class _Cont>
inline channel<void, _Cont>::channel(channel&& __c)
  : __channel_base(static_cast<__channel_base&&>(__c)), _M_buffered(__c._M_buffered)
{
  __c._M_buffered = 0;
}

template <class _Cont>
inline channel<void, _Cont>& channel<void, _Cont>::operator=(channel&& __c)
{
  _Move_from(static_cast<__channel_base&&>(__c));
  _M_buffered = __c._M_buffered;
  __c._M_buffered = 0;
  return *this;
}

template <class _Cont>
inline channel<void, _Cont>::~channel()
{
}

template <class _Cont>
inline execution_context& channel<void, _Cont>::context()
{
  return _Context();
}

template <class _Cont>
inline size_t channel<void, _Cont>::capacity() const
{
  return _Capacity();
}

template <class _Cont>
inline bool channel<void, _Cont>::is_open() const
{
  return _Is_open();
}

template <class _Cont>
inline void channel<void, _Cont>::reset()
{
  _Reset();
  _M_buffered = 0;
}

template <class _Cont>
inline void channel<void, _Cont>::close()
{
  _Close();
}

template <class _Cont>
inline void channel<void, _Cont>::cancel()
{
  _Cancel();
}

template <class _Cont>
inline bool channel<void, _Cont>::ready() const
{
  return _Ready();
}

template <class _Cont>
void channel<void, _Cont>::put()
{
  error_code __ec;
  this->put(__ec);
  if (__ec)
    throw system_error(__ec);
}

template <class _Cont>
void channel<void, _Cont>::put(error_code& __ec)
{
  switch (_M_put_state)
  {
  case __channel_state::block:
    {
      __ec = make_error_code(errc::operation_would_block);
      break;
    }
  case __channel_state::buffer:
    {
      ++_M_buffered;
      _M_get_state = __channel_state::buffer;
      if (_M_buffered == _Capacity())
        _M_put_state = __channel_state::block;
      __ec.clear();
      break;
    }
  case __channel_state::waiter:
    {
      __channel_op* __getter = _M_waiters._Front();
      _M_waiters._Pop();
      if (_M_waiters._Empty())
        _M_put_state = _Capacity() ? __channel_state::buffer : __channel_state::block;
      __ec.clear();
      __getter->_Complete();
      break;
    }
  case __channel_state::closed:
  default:
    {
      __ec = make_error_code(errc::broken_pipe);
      break;
    }
  }
}

template <class _Cont> template <class _CompletionToken>
auto channel<void, _Cont>::put(_CompletionToken&& __token)
{
  typedef handler_type_t<_CompletionToken, void(error_code)> _Handler;
  async_completion<_CompletionToken, void(error_code)> __completion(__token);

  auto __allocator(get_associated_allocator(__completion.handler));
  auto __op(_Allocate_small_block<_Op<_Handler>>(__allocator, std::move(__completion.handler)));

  _Start_put(__op.get());
  __op.release();

  return __completion.result.get();
}

template <class _Cont>
void channel<void, _Cont>::get()
{
  error_code __ec;
  this->get(__ec);
  if (__ec)
    throw system_error(__ec);
}

template <class _Cont>
void channel<void, _Cont>::get(error_code& __ec)
{
  switch (_M_get_state)
  {
  case __channel_state::block:
    {
      __ec = make_error_code(errc::operation_would_block);
      return;
    }
  case __channel_state::buffer:
    {
      if (__channel_op* __putter = _M_waiters._Front())
      {
        _M_waiters._Pop();
        __putter->_Complete();
      }
      else
      {
        --_M_buffered;
        if (_M_buffered == 0)
          _M_get_state = (_M_put_state == __channel_state::closed) ? __channel_state::closed : __channel_state::block;
        _M_put_state = (_M_put_state == __channel_state::closed) ? __channel_state::closed : __channel_state::buffer;
      }
      __ec.clear();
      return;
    }
  case __channel_state::waiter:
    {
      __channel_op* __putter = _M_waiters._Front();
      _M_waiters._Pop();
      if (_M_waiters._Front() == 0)
        _M_get_state = (_M_put_state == __channel_state::closed) ? __channel_state::closed : __channel_state::block;
      __putter->_Complete();
      __ec.clear();
      return;
    }
  case __channel_state::closed:
  default:
    {
      __ec = make_error_code(errc::broken_pipe);
      return;
    }
  }
}

template <class _Cont> template <class _CompletionToken>
auto channel<void, _Cont>::get(_CompletionToken&& __token)
{
  typedef handler_type_t<_CompletionToken, void(error_code)> _Handler;
  async_completion<_CompletionToken, void(error_code)> __completion(__token);

  auto __allocator(get_associated_allocator(__completion.handler));
  auto __op(_Allocate_small_block<_Op<_Handler>>(__allocator, std::move(__completion.handler)));

  _Start_get(__op.get());
  __op.release();

  return __completion.result.get();
}

template <class _Cont>
void channel<void, _Cont>::_Start_put(__channel_op* __putter)
{
  switch (_M_put_state)
  {
  case __channel_state::block:
    {
      _M_waiters._Push(__putter);
      if (_M_get_state == __channel_state::block)
        _M_get_state = __channel_state::waiter;
      return;
    }
  case __channel_state::buffer:
    {
      ++_M_buffered;
      _M_get_state = __channel_state::buffer;
      if (_M_buffered == _Capacity())
        _M_put_state = __channel_state::block;
      __putter->_Complete();
      break;
    }
  case __channel_state::waiter:
    {
      __channel_op* __getter = _M_waiters._Front();
      _M_waiters._Pop();
      if (_M_waiters._Empty())
        _M_put_state = _Capacity() ? __channel_state::buffer : __channel_state::block;
      __getter->_Complete();
      __putter->_Complete();
      break;
    }
  case __channel_state::closed:
  default:
    {
      __putter->_Close();
      break;
    }
  }
}

template <class _Cont>
void channel<void, _Cont>::_Start_get(__channel_op* __getter)
{
  switch (_M_get_state)
  {
  case __channel_state::block:
    {
      _M_waiters._Push(__getter);
      if (_M_put_state != __channel_state::closed)
        _M_put_state = __channel_state::waiter;
      return;
    }
  case __channel_state::buffer:
    {
      if (__channel_op* __putter = _M_waiters._Front())
      {
        _M_waiters._Pop();
        __putter->_Complete();
      }
      else
      {
        --_M_buffered;
        if (_M_buffered == 0)
          _M_get_state = (_M_put_state == __channel_state::closed) ? __channel_state::closed : __channel_state::block;
        _M_put_state = (_M_put_state == __channel_state::closed) ? __channel_state::closed : __channel_state::buffer;
      }
      __getter->_Complete();
      break;
    }
  case __channel_state::waiter:
    {
      __channel_op* __putter = _M_waiters._Front();
      _M_waiters._Pop();
      if (_M_waiters._Front() == 0)
        _M_get_state = (_M_put_state == __channel_state::closed) ? __channel_state::closed : __channel_state::block;
      __putter->_Complete();
      __getter->_Complete();
      break;
    }
  case __channel_state::closed:
  default:
    {
      __getter->_Close();
      break;
    }
  }
}

} // inline namespace concurrency_v1
} // namespace experimental
} // namespace std

#endif
