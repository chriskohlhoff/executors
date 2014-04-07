//
// tail_context.h
// ~~~~~~~~~~~~~~
// Tail-call context implementation.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_TAIL_CONTEXT_H
#define EXECUTORS_EXPERIMENTAL_BITS_TAIL_CONTEXT_H

#include <utility>
#include <experimental/bits/executor_wrapper.h>
#include <experimental/bits/function_traits.h>

namespace std {
namespace experimental {

template <class _Executor> template <class _OtherExecutor>
basic_tail_context<_Executor>::basic_tail_context(basic_tail_context<_OtherExecutor>&& __c)
  : _M_executor(__c._M_executor), _M_impl(__c._M_impl)
{
}

class __tail_context
{
public:
  virtual ~__tail_context() {}
};

template <class _Signature>
class __tail_context_impl_base;

template <class _R, class... _Args>
class __tail_context_impl_base<_R(_Args...)>
  : public __tail_context
{
public:
  virtual _R operator()(_Args... __args) = 0;
};

template <class _Tail, class _Signature>
class __tail_context_impl;

template <class _Tail, class _R, class... _Args>
class __tail_context_impl<_Tail, _R(_Args...)>
  : public __tail_context_impl_base<_R(_Args...)>
{
public:
  template <class _T> explicit __tail_context_impl(_T&& __t)
    : _M_tail(forward<_T>(__t)) {}

  virtual _R operator()(_Args... __args)
  {
    return std::move(_M_tail)(forward<_Args>(__args)...);
  }

private:
  _Tail _M_tail;
};

template <class _Executor, class _Signature>
struct __tail_context_handler
{
  __tail_context_handler(basic_tail_context<_Executor>&& __c)
    : _M_executor(__c._M_executor),
      _M_impl(static_cast<__tail_context_impl_base<_Signature>*>(__c._M_impl.release()))
  {
  }

  template <class... _Args> void operator()(_Args&&... __args) &&
  {
    (*_M_impl)(forward<_Args>(__args)...);
  }

  _Executor _M_executor;
  unique_ptr<__tail_context_impl_base<_Signature>> _M_impl;
};

template <class _Executor, class _Signature>
inline auto make_executor(const __tail_context_handler<_Executor, _Signature>& __h)
{
  return __h._M_executor;
}

template <class _Executor, class _Signature>
class async_result<__tail_context_handler<_Executor, _Signature>>
{
public:
  typedef tail_result<_Signature> type;

  async_result(__tail_context_handler<_Executor, _Signature>&)
  {
  }

  async_result(const async_result&) = delete;
  async_result& operator=(const async_result&) = delete;

  type get()
  {
    return type();
  }
};

template <class _Executor, class _R, class... _Args>
struct handler_type<basic_tail_context<_Executor>, _R(_Args...)>
{
  typedef __tail_context_handler<_Executor, _R(_Args...)> type;
};

template <class _Executor, class _Func>
struct __tail_context_launcher
{
  typename _Executor::work _M_work;
  _Func _M_func;
  unique_ptr<__tail_context> _M_tail;

  template <class _F> __tail_context_launcher(_F&& __f)
    : _M_work(make_executor(__f).make_work()), _M_func(forward<_F>(__f))
  {
  }

  template <class _F> __tail_context_launcher(
    executor_arg_t, const _Executor& __e, _F&& __f)
      : _M_work(make_executor(__e).make_work()), _M_func(forward<_F>(__f))
  {
  }

  template <class _F, class _C> __tail_context_launcher(
    true_type, const typename _Executor::work& __w, _F&& __f, _C&& __c)
      : _M_work(__w), _M_func(forward<_F>(__f)), _M_tail(forward<_C>(__c))
  {
  }

  template <class... _Args> void operator()(_Args&&... __args) &&
  {
    std::move(_M_func)(forward<_Args>(__args)...,
      basic_tail_context<_Executor>(make_executor(_M_work), std::move(_M_tail)));
  }
};

template <class _ExecutorTo, class _Func, class _ExecutorFrom>
struct uses_executor<__tail_context_launcher<_ExecutorTo, _Func>, _ExecutorFrom>
  : is_convertible<_ExecutorFrom, _ExecutorTo> {};

template <class _T, class = void>
struct __has_tail_call : false_type {};

template <class _T>
struct __has_tail_call<_T,
  typename enable_if<is_convertible<__last_argument_t<__signature_t<_T>>,
    tail_context>::value>::type> : true_type {};

template <class _Func, class _R, class... _Args>
struct handler_type<_Func, _R(_Args...),
  typename enable_if<__has_tail_call<typename decay<_Func>::type>::value
    && !__is_executor_wrapper<typename decay<_Func>::type>::value>::type>
{
  typedef typename decay<_Func>::type _DecayFunc;
  typedef __last_argument_t<__signature_t<_DecayFunc>> _TailContext;
  typedef decltype(make_executor(declval<_TailContext>())) _Executor;
  typedef __tail_context_launcher<_Executor, _DecayFunc> type;
};

template <class _Result>
struct __tail_result_signature
{
  static_assert(!sizeof(_Result*),
    "A tail_context function must return the result of the tail call operation.");
};

template <class _Signature>
struct __tail_result_signature<tail_result<_Signature>>
{
  typedef _Signature signature;
};

template <class _Executor, class _Func>
struct continuation_of<__tail_context_launcher<_Executor, _Func>>
{
  typedef typename decay<_Func>::type _DecayFunc;
  typedef __result_t<__signature_t<_DecayFunc>> _Result;
  typedef typename __tail_result_signature<_Result>::signature signature;

  template <class _F, class _Continuation>
  static auto chain(_F&& __f, _Continuation&& __c)
  {
    typedef typename decay<_Continuation>::type _DecayContinuation;
    typedef __tail_context_impl<_DecayContinuation, signature> _Tail;
    __tail_context_launcher<_Executor, _DecayFunc> __launcher(forward<_F>(__f));
    __launcher._M_tail.reset(new _Tail(forward<_Continuation>(__c)));
    return __launcher;
  }
};

} // namespace experimental
} // namespace std

#endif
