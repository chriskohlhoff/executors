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

namespace std {
namespace experimental {

struct __empty_function_void0 { void operator()() {} };

template <class _Result, class _Handler>
struct __invoke_with_result
{
  _Result _M_result;
  _Handler _M_handler;

  void operator()()
  {
    _M_handler(std::move(_M_result));
  }
};

template <class _Result1, class _Result2, class _Handler>
struct __invoke_with_result_2
{
  _Result1 _M_result1;
  _Result2 _M_result2;
  _Handler _M_handler;

  void operator()()
  {
    _M_handler(std::move(_M_result1), std::move(_M_result2));
  }
};

template <class _Func, class _FuncSignature, class _Handler>
struct __invoker;

template <class _Func, class _FuncResult, class... _FuncArgs, class _Handler>
struct __invoker<_Func, _FuncResult(_FuncArgs...), _Handler>
{
  _Func _M_func;
  _Handler _M_handler;
  typename decltype(make_executor(declval<_Handler>()))::work _M_handler_work;

  void operator()(_FuncArgs... __args)
  {
    this->_Invoke(is_same<void, _FuncResult>(), forward<_FuncArgs>(__args)...);
  }

private:
  void _Invoke(true_type, _FuncArgs... __args)
  {
    _M_func(forward<_FuncArgs>(__args)...);
    make_executor(_M_handler_work).dispatch(std::move(_M_handler));
  }

  void _Invoke(false_type, _FuncArgs... __args)
  {
    make_executor(_M_handler_work).dispatch(
      __invoke_with_result<_FuncResult, _Handler>{
        _M_func(forward<_FuncArgs>(__args)...), std::move(_M_handler)});
  }
};

struct __invoker_func_executor
{
  template <class _Func, class _FuncSignature, class _Handler>
  static auto _Get(const __invoker<_Func, _FuncSignature, _Handler>& __i)
  {
    return make_executor(__i._M_func);
  }
};

struct __invoker_handler_executor
{
  template <class _Func, class _FuncSignature, class _Handler>
  static auto _Get(const __invoker<_Func, _FuncSignature, _Handler>& __i)
  {
    return make_executor(__i._M_handler);
  }
};

template <class _Func, class _FuncSignature, class _Handler>
inline auto make_executor(const __invoker<_Func, _FuncSignature, _Handler>& __i)
{
  typedef decltype(make_executor(__i._M_func)) _FuncExecutor;
  return conditional<is_same<_FuncExecutor, system_executor>::value,
    __invoker_handler_executor, __invoker_func_executor>::type::_Get(__i);
}

} // namespace experimental
} // namespace std

#endif
