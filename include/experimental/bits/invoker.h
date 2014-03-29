//
// invoker.h
// ~~~~~~~~~
// Function objects used to implement post, dispatch, etc.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_INVOKER_H
#define EXECUTORS_EXPERIMENTAL_BITS_INVOKER_H

#include <experimental/bits/tuple_utils.h>

namespace std {
namespace experimental {

template <class _Handler>
class __invoker
{
public:
  template <class _H> explicit __invoker(_H&& __h)
    : _M_handler(forward<_H>(__h)),
      _M_handler_work(make_executor(_M_handler).make_work())
  {
  }

  template <class... _Args> void operator()(_Args&&... __args)
  {
    make_executor(_M_handler_work).dispatch(
      _Make_tuple_invoker(std::move(_M_handler), forward<_Args>(__args)...));
  }

private:
  _Handler _M_handler;
  typename decltype(make_executor(declval<_Handler>()))::work _M_handler_work;
};

struct __invoker_func_executor
{
  template <class _Func, class _Handler>
  static auto _Get(_Func& __f, _Handler&)
  {
    return make_executor(__f);
  }
};

struct __invoker_handler_executor
{
  template <class _Func, class _Handler>
  static auto _Get(_Func&, _Handler& __h)
  {
    return make_executor(__h);
  }
};

template <class _Func, class _Handler>
inline auto __make_invoker_executor(_Func& __f, _Handler& __h)
{
  typedef decltype(make_executor(__f)) _FuncExecutor;
  return conditional<is_same<_FuncExecutor, system_executor>::value,
    __invoker_handler_executor, __invoker_func_executor>::type::_Get(__f, __h);
}

} // namespace experimental
} // namespace std

#endif
