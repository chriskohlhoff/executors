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

namespace std {
namespace experimental {

template <class _Result>
class __coinvoker_result
{
public:
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

  template <class _T> void _Set_value(_T&& __t)
  {
    assert(!_M_has_value);
    new (&_M_value) _Result(forward<_T>(__t));
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

template <>
class __coinvoker_result<void>
{
public:
  __coinvoker_result(const __coinvoker_result&) = delete;
  __coinvoker_result& operator=(const __coinvoker_result&) = delete;
  __coinvoker_result() {}
  ~__coinvoker_result() {}
  void _Set_value() {}
  void _Get_value() {}
};

template <class _Result1, class _Result2, class _Handler>
class __coinvoker_handler
{
public:
  template <class _H> explicit __coinvoker_handler(_H&& __h)
    : _M_pending(0), _M_handler(forward<_H>(__h)),
      _M_handler_work(make_executor(_M_handler).make_work())
  {
  }

  template <class... _Args> void _Set_result(integral_constant<int, 1>, _Args&&... __args)
  {
    _M_result1._Set_value(forward<_Args>(__args)...);
  }

  template <class... _Args> void _Set_result(integral_constant<int, 2>, _Args&&... __args)
  {
    _M_result2._Set_value(forward<_Args>(__args)...);
  }

  void _Prime()
  {
    _M_pending = 2;
  }

  void _Complete()
  {
    if (--_M_pending == 0)
    {
      unique_ptr<__coinvoker_handler> __p(this);
      make_executor(_M_handler_work).dispatch(_Make_handler(_M_result1, _M_result2));
    }
  }

  void _Release()
  {
    if (--_M_pending == 0)
      delete this;
  }

private:
  friend struct __coinvoker_handler_executor;

  template <class _T, class _U>
  auto _Make_handler(__coinvoker_result<_T>& __r1, __coinvoker_result<_U>& __r2)
  {
    return __invoke_with_result_2<_T, _U, _Handler>{
      __r1._Get_value(), __r2._Get_value(), std::move(_M_handler)};
  }

  template <class _T>
  auto _Make_handler(__coinvoker_result<_T>& __r1, __coinvoker_result<void>&)
  {
    return __invoke_with_result<_T, _Handler>{__r1._Get_value(), std::move(_M_handler)};
  }

  template <class _T>
  auto _Make_handler(__coinvoker_result<void>&, __coinvoker_result<_T>& __r2)
  {
    return __invoke_with_result<_T, _Handler>{__r2._Get_value(), std::move(_M_handler)};
  }

  _Handler _Make_handler(__coinvoker_result<void>&, __coinvoker_result<void>&)
  {
    return std::move(_M_handler);
  }

  __coinvoker_result<_Result1> _M_result1;
  __coinvoker_result<_Result2> _M_result2;
  atomic<int> _M_pending;
  _Handler _M_handler;
  typename decltype(make_executor(declval<_Handler>()))::work _M_handler_work;
};

template <int _Tag, class _Result1, class _Result2, class _Handler>
class __coinvoker
{
public:
  __coinvoker(const __coinvoker&) = delete;
  __coinvoker& operator=(const __coinvoker&) = delete;

  explicit __coinvoker(__coinvoker_handler<_Result1, _Result2, _Handler>* __h)
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
    _M_handler->_Set_result(integral_constant<int, _Tag>(), forward<_Args>(__args)...);
    this->_Complete();
  }

  auto _Get_executor() const
  {
    return make_executor(_M_handler->_M_handler);
  }

private:
  void _Complete()
  {
    __coinvoker_handler<_Result1, _Result2, _Handler>* __h = _M_handler;
    _M_handler = 0;
    __h->_Complete();
  }

  __coinvoker_handler<_Result1, _Result2, _Handler>* _M_handler;
};

template <int _Tag, class _Result1, class _Result2, class _Handler>
inline auto make_executor(const __coinvoker<_Tag, _Result1, _Result2, _Handler>& __i)
{
  return __i._Get_executor();
}

} // namespace experimental
} // namespace std

#endif
