//
// coinvoker.h
// ~~~~~~~~~~~
// Function objects used to implement copost, codispatch, etc.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_COINVOKER_H
#define EXECUTORS_EXPERIMENTAL_BITS_COINVOKER_H

#include <atomic>
#include <cassert>
#include <memory>
#include <type_traits>
#include <experimental/bits/coinvoker.h>
#include <experimental/bits/tuple_utils.h>

namespace std {
namespace experimental {

template <class _Signature>
class __coinvoker_result
{
public:
  typedef __args_tuple_t<_Signature> _Result;

  __coinvoker_result(const __coinvoker_result&) = delete;
  __coinvoker_result& operator=(const __coinvoker_result&) = delete;

  __coinvoker_result()
    : _M_has_value(false)
  {
  }

  ~__coinvoker_result()
  {
    if (_M_has_value)
    {
      _Result* __v = static_cast<_Result*>(static_cast<void*>(&_M_value));
      __v->~_Result();
    }
  }

  template <class... _Args> void _Set_value(_Args&&... __args)
  {
    assert(!_M_has_value);
    new (&_M_value) _Result(forward<_Args>(__args)...);
    _M_has_value = true;
  }

  _Result _Get_value()
  {
    assert(_M_has_value);
    return std::move(*static_cast<_Result*>(static_cast<void*>(&_M_value)));
  }

private:
  typename aligned_storage<sizeof(_Result)>::type _M_value;
  bool _M_has_value;
};

template <class _Handler, class... _Signatures>
class __coinvoker_handler
{
public:
  template <class _H> explicit __coinvoker_handler(_H&& __h)
    : _M_pending(0), _M_handler(forward<_H>(__h)),
      _M_handler_work(make_executor(_M_handler).make_work())
  {
  }

  template <size_t _Index, class... _Args> void _Set_result(_Args&&... __args)
  {
    get<_Index>(_M_results)._Set_value(forward<_Args>(__args)...);
  }

  void _Prime()
  {
    _M_pending = sizeof...(_Signatures);
  }

  void _Complete()
  {
    if (--_M_pending == 0)
    {
      unique_ptr<__coinvoker_handler> __p(this);
      _Dispatch(typename _Make_index_sequence<sizeof...(_Signatures)>::_Type());
    }
  }

  void _Release()
  {
    if (--_M_pending == 0)
      delete this;
  }

private:
  friend struct __coinvoker_handler_executor;

  template <size_t... _Index>
  void _Dispatch(_Index_sequence<_Index...>)
  {
    make_executor(_M_handler_work).dispatch(_Make_tuple_invoker(
      std::move(_M_handler), std::tuple_cat(get<_Index>(_M_results)._Get_value()...)));
  }

  tuple<__coinvoker_result<_Signatures>...> _M_results;
  atomic<int> _M_pending;
  _Handler _M_handler;
  typename decltype(make_executor(declval<_Handler>()))::work _M_handler_work;
};

template <size_t _Index, class _Handler, class... _Signatures>
class __coinvoker
{
public:
  __coinvoker(const __coinvoker&) = delete;
  __coinvoker& operator=(const __coinvoker&) = delete;

  explicit __coinvoker(__coinvoker_handler<_Handler, _Signatures...>* __h)
    : _M_handler(__h)
  {
  }

  __coinvoker(__coinvoker&& __f)
    : _M_handler(__f._M_handler)
  {
    __f._M_handler = 0;
  }

  ~__coinvoker()
  {
    if (_M_handler)
      _M_handler->_Release();
  }

  template <class... _Args> void operator()(_Args&&... __args)
  {
    _M_handler->template _Set_result<_Index>(forward<_Args>(__args)...);
    this->_Complete();
  }

  decltype(make_executor(declval<_Handler>())) _Get_executor() const
  {
    return make_executor(_M_handler->_M_handler);
  }

private:
  void _Complete()
  {
    auto* __h = _M_handler;
    _M_handler = 0;
    __h->_Complete();
  }

  __coinvoker_handler<_Handler, _Signatures...>* _M_handler;
};

template <size_t _Index, class _Handler, class... _Signatures>
inline auto make_executor(const __coinvoker<_Index, _Handler, _Signatures...>& __i)
{
  return __i._Get_executor();
}

} // namespace experimental
} // namespace std

#endif
