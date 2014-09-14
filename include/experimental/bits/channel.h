//
// channel.h
// ~~~~~~~~~
// A channel of values.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_CHANNEL_H
#define EXECUTORS_EXPERIMENTAL_BITS_CHANNEL_H

#include <experimental/executor>
#include <experimental/memory>
#include <experimental/bits/tuple_utils.h>

namespace std {
namespace experimental {
inline namespace concurrency_v1 {

template <class _T, class _Cont>
class channel<_T, _Cont>::_Op
  : public __channel_op
{
public:
  value_type _Get_value()
  {
    if (!_M_has_value)
      return value_type();
    return std::move(*static_cast<value_type*>(static_cast<void*>(&_M_value)));
  }

  template <class _U> void _Set_value(_U&& __u)
  {
    new (&_M_value) value_type(forward<_U>(__u));
    _M_has_value = true;
  }

protected:
  _Op() {}

  template <class _U>
  _Op(_U&& __u)
  {
    this->_Set_value(forward<_U>(__u));
  }

  ~_Op()
  {
    if (_M_has_value)
    {
      value_type* __v = static_cast<value_type*>(static_cast<void*>(&_M_value));
      __v->~value_type();
    }
  }

private:
  typename aligned_storage<sizeof(value_type)>::type _M_value;
};

template <class _T, class _Cont> template <class _Handler>
class channel<_T, _Cont>::_PutOp
  : public _Op
{
public:
  template <class _U, class _H> explicit _PutOp(_U&& __u, _H&& __h)
    : _Op(forward<_U>(__u)), _M_handler(forward<_H>(__h)),
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
  typedef associated_executor_t<_Handler> _Executor;
  executor_work<_Executor> _M_work;
};

template <class _T, class _Cont> template <class _Handler>
class channel<_T, _Cont>::_GetOp
  : public _Op
{
public:
  template <class _H> explicit _GetOp(_H&& __h)
    : _M_handler(forward<_H>(__h)), _M_work(get_associated_executor(_M_handler))
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
    auto __i(_Make_tuple_invoker(std::move(_M_handler), __ec, this->_Get_value()));
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
  typedef associated_executor_t<_Handler> _Executor;
  executor_work<_Executor> _M_work;
};

template <class _T, class _Cont>
inline channel<_T, _Cont>::channel()
  : __channel_base(system_executor().context(), 0)
{
}

template <class _T, class _Cont>
inline channel<_T, _Cont>::channel(size_t __capacity)
  : __channel_base(system_executor().context(), __capacity)
{
}

template <class _T, class _Cont>
inline channel<_T, _Cont>::channel(execution_context& __c)
  : __channel_base(__c, 0)
{
}

template <class _T, class _Cont>
inline channel<_T, _Cont>::channel(execution_context& __c, size_t __capacity)
  : __channel_base(__c, __capacity)
{
}

template <class _T, class _Cont>
inline channel<_T, _Cont>::channel(channel&& __c)
  : __channel_base(static_cast<__channel_base&&>(__c)), _M_buffer(__c._M_buffer)
{
}

template <class _T, class _Cont>
channel<_T, _Cont>& channel<_T, _Cont>::operator=(channel&& __c)
{
  _Move_from(static_cast<__channel_base&&>(__c));
  _M_buffer = std::move(__c._M_buffer);
  return *this;
}

template <class _T, class _Cont>
inline channel<_T, _Cont>::~channel()
{
}

template <class _T, class _Cont>
inline execution_context& channel<_T, _Cont>::context()
{
  return _Context();
}

template <class _T, class _Cont>
inline size_t channel<_T, _Cont>::capacity() const
{
  return _Capacity();
}

template <class _T, class _Cont>
inline bool channel<_T, _Cont>::is_open() const
{
  return _Is_open();
}

template <class _T, class _Cont>
inline void channel<_T, _Cont>::reset()
{
  _Reset();
  _M_buffer.clear();
}

template <class _T, class _Cont>
inline void channel<_T, _Cont>::close()
{
  _Close();
}

template <class _T, class _Cont>
inline void channel<_T, _Cont>::cancel()
{
  _Cancel();
}

template <class _T, class _Cont>
inline bool channel<_T, _Cont>::ready() const
{
  return _Ready();
}

template <class _T, class _Cont> template <class _U>
void channel<_T, _Cont>::put(_U&& __u)
{
  error_code __ec;
  this->put(forward<_U>(__u), __ec);
  if (__ec)
    throw system_error(__ec);
}

template <class _T, class _Cont> template <class _U>
void channel<_T, _Cont>::put(_U&& __u, error_code& __ec)
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
      _M_buffer.push_back(forward<_U>(__u));
      _M_get_state = __channel_state::buffer;
      if (_M_buffer.size() == _Capacity())
        _M_put_state = __channel_state::block;
      __ec.clear();
      break;
    }
  case __channel_state::waiter:
    {
      _Op* __getter = static_cast<_Op*>(_M_waiters._Front());
      __getter->_Set_value(forward<_U>(__u));
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

template <class _T, class _Cont> template <class _U, class _CompletionToken>
auto channel<_T, _Cont>::put(_U&& __u, _CompletionToken&& __token)
{
  typedef handler_type_t<_CompletionToken, void(error_code)> _Handler;
  async_completion<_CompletionToken, void(error_code)> __completion(__token);

  auto __allocator(get_associated_allocator(__completion.handler));
  auto __op(_Allocate_small_block<_PutOp<_Handler>>(__allocator,
    forward<_U>(__u), std::move(__completion.handler)));

  _Start_put(__op.get());
  __op.release();

  return __completion.result.get();
}

template <class _T, class _Cont>
typename channel<_T, _Cont>::value_type channel<_T, _Cont>::get()
{
  error_code __ec;
  _T __result(this->get(__ec));
  if (__ec)
    throw system_error(__ec);
  return __result;
}

template <class _T, class _Cont>
typename channel<_T, _Cont>::value_type channel<_T, _Cont>::get(error_code& __ec)
{
  switch (_M_get_state)
  {
  case __channel_state::block:
    {
      __ec = make_error_code(errc::operation_would_block);
      return typename _Cont::value_type();
    }
  case __channel_state::buffer:
    {
      typename _Cont::value_type __tmp(_M_buffer.front());
      if (_Op* __putter = static_cast<_Op*>(_M_waiters._Front()))
      {
        _M_buffer.push_back(__putter->_Get_value());
        _M_buffer.pop_front();
        _M_waiters._Pop();
        __putter->_Complete();
      }
      else
      {
        _M_buffer.pop_front();
        if (_M_buffer.size() == 0)
          _M_get_state = (_M_put_state == __channel_state::closed) ? __channel_state::closed : __channel_state::block;
        _M_put_state = (_M_put_state == __channel_state::closed) ? __channel_state::closed : __channel_state::buffer;
      }
      __ec.clear();
      return __tmp;
    }
  case __channel_state::waiter:
    {
      _Op* __putter = static_cast<_Op*>(_M_waiters._Front());
      value_type __tmp(__putter->_Get_value());
      _M_waiters._Pop();
      if (_M_waiters._Front() == 0)
        _M_get_state = (_M_put_state == __channel_state::closed) ? __channel_state::closed : __channel_state::block;
      __putter->_Complete();
      __ec.clear();
      return __tmp;
    }
  case __channel_state::closed:
  default:
    {
      __ec = make_error_code(errc::broken_pipe);
      return typename _Cont::value_type();
    }
  }
}

template <class _T, class _Cont> template <class _CompletionToken>
auto channel<_T, _Cont>::get(_CompletionToken&& __token)
{
  typedef handler_type_t<_CompletionToken, void(error_code, _T)> _Handler;
  async_completion<_CompletionToken, void(error_code, _T)> __completion(__token);

  auto __allocator(get_associated_allocator(__completion.handler));
  auto __op(_Allocate_small_block<_GetOp<_Handler>>(
    __allocator, std::move(__completion.handler)));

  _Start_get(__op.get());
  __op.release();

  return __completion.result.get();
}

template <class _T, class _Cont>
void channel<_T, _Cont>::_Start_put(_Op* __putter)
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
      _M_buffer.push_back(__putter->_Get_value());
      _M_get_state = __channel_state::buffer;
      if (_M_buffer.size() == _Capacity())
        _M_put_state = __channel_state::block;
      __putter->_Complete();
      break;
    }
  case __channel_state::waiter:
    {
      _Op* __getter = static_cast<_Op*>(_M_waiters._Front());
      __getter->_Set_value(__putter->_Get_value());
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

template <class _T, class _Cont>
void channel<_T, _Cont>::_Start_get(_Op* __getter)
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
      __getter->_Set_value(_M_buffer.front());
      if (_Op* __putter = static_cast<_Op*>(_M_waiters._Front()))
      {
        _M_buffer.push_back(__putter->_Get_value());
        _M_buffer.pop_front();
        _M_waiters._Pop();
        __putter->_Complete();
      }
      else
      {
        _M_buffer.pop_front();
        if (_M_buffer.size() == 0)
          _M_get_state = (_M_put_state == __channel_state::closed) ? __channel_state::closed : __channel_state::block;
        _M_put_state = (_M_put_state == __channel_state::closed) ? __channel_state::closed : __channel_state::buffer;
      }
      __getter->_Complete();
      break;
    }
  case __channel_state::waiter:
    {
      _Op* __putter = static_cast<_Op*>(_M_waiters._Front());
      __getter->_Set_value(__putter->_Get_value());
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
