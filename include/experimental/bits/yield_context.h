//
// yield_context.h
// ~~~~~~~~~~~~~~~
// Stackful coroutine implementation.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_YIELD_CONTEXT_H
#define EXECUTORS_EXPERIMENTAL_BITS_YIELD_CONTEXT_H

#include <exception>
#include <system_error>
#include <tuple>
#include <utility>
#include <experimental/bits/exception_ptr_executor.h>
#include <experimental/bits/executor_wrapper.h>
#include <experimental/bits/function_traits.h>
#include <experimental/bits/invoker.h>
#include <experimental/bits/tuple_utils.h>

#ifndef EXECUTORS_NO_BOOST
# include <boost/coroutine/all.hpp>
#endif

namespace std {
namespace experimental {
inline namespace concurrency_v1 {

template <class _Executor> template <class _OtherExecutor>
basic_yield_context<_Executor>::basic_yield_context(const basic_yield_context<_OtherExecutor>& __c)
  : _M_executor(__c._M_executor), _M_callee(__c._M_callee),
    _M_caller(__c._M_caller), _M_error_code(__c._M_error_code)
{
}

template <class _Executor>
basic_yield_context<_Executor> basic_yield_context<_Executor>::operator[](error_code& __ec) const
{
  basic_yield_context<_Executor> __c(*this);
  __c._M_error_code = &__ec;
  return __c;
}

template <class _Executor>
inline typename basic_yield_context<_Executor>::executor_type
basic_yield_context<_Executor>::get_executor() const noexcept
{
  return _M_executor;
}

struct __yield_context_caller
{
#ifdef EXECUTORS_NO_BOOST
  struct _Coro { void operator()(){} } _M_coro;
#else
  boost::coroutines::pull_coroutine<void>& _M_coro;
#endif
};

struct __yield_context_callee
{
#ifdef EXECUTORS_NO_BOOST
  struct _Coro { void operator()(){} } _M_coro;
#else
  boost::coroutines::push_coroutine<void> _M_coro;
#endif
  atomic_flag _M_lock = ATOMIC_FLAG_INIT;

  class _Lock_guard
  {
  public:
    _Lock_guard(const _Lock_guard&) = delete;
    _Lock_guard& operator=(const _Lock_guard&) = delete;

    explicit _Lock_guard(__yield_context_callee& __c) : _M_lock(__c._M_lock)
    {
      while (_M_lock.test_and_set(memory_order_acquire)) {}
    }

    ~_Lock_guard()
    {
      _M_lock.clear(memory_order_release);
    }

  private:
    atomic_flag& _M_lock;
  };
};

template <class... _Values>
struct __yield_context_result
{
  exception_ptr _M_exception;
  tuple<_Values...> _M_value;
  atomic_flag _M_ready = ATOMIC_FLAG_INIT;

  template <class... _Args>
  void _Set_value(_Args&&... __args)
  {
    _M_value = std::make_tuple(forward<_Args>(__args)...);
  }

  decltype(_Tuple_get(declval<tuple<_Values...>>())) _Get()
  {
    if (_M_exception)
      rethrow_exception(_M_exception);
    return _Tuple_get(std::move(_M_value));
  }
};

template <class _Executor, class... _Values>
struct __yield_context_handler
{
  typedef __exception_ptr_executor<_Executor> executor_type;
  typedef __yield_context_result<_Values...> _Result;

  __yield_context_handler(basic_yield_context<_Executor> __c)
    : _M_executor(__c._M_executor), _M_callee(__c._M_callee.lock()),
      _M_caller(__c._M_caller), _M_result(nullptr)
  {
  }

  executor_type get_executor() const noexcept
  {
    return executor_type(&_M_result->_M_exception, _M_executor);
  }

  template <class... _Args> void operator()(_Args&&... __args)
  {
    _M_result->_Set_value(forward<_Args>(__args)...);
    if (_M_result->_M_ready.test_and_set())
    {
      typename __yield_context_callee::_Lock_guard __lock(*_M_callee);
      _M_callee->_M_coro();
    }
  }

  _Executor _M_executor;
  shared_ptr<__yield_context_callee> _M_callee;
  __yield_context_caller* _M_caller;
  __yield_context_result<_Values...>* _M_result;
};

template <class _Executor, class... _Values>
struct __yield_context_handler<_Executor, error_code, _Values...>
{
  typedef __exception_ptr_executor<_Executor> executor_type;
  typedef __yield_context_result<_Values...> _Result;

  __yield_context_handler(basic_yield_context<_Executor> __c)
    : _M_executor(__c._M_executor), _M_callee(__c._M_callee.lock()),
      _M_caller(__c._M_caller), _M_result(nullptr), _M_error_code(__c._M_error_code)
  {
  }

  executor_type get_executor() const noexcept
  {
    return executor_type(&_M_result->_M_exception, _M_executor);
  }

  template <class... _Args> void operator()(const error_code& __e, _Args&&... __args)
  {
    if (_M_error_code)
      *_M_error_code = __e;
    else if (__e)
      _M_result->_M_exception = make_exception_ptr(system_error(__e));
    _M_result->_Set_value(forward<_Args>(__args)...);
    if (_M_result->_M_ready.test_and_set())
    {
      typename __yield_context_callee::_Lock_guard __lock(*_M_callee);
      _M_callee->_M_coro();
    }
  }

  _Executor _M_executor;
  shared_ptr<__yield_context_callee> _M_callee;
  __yield_context_caller* _M_caller;
  __yield_context_result<_Values...>* _M_result;
  error_code* _M_error_code;
};

template <class _Executor, class... _Values>
struct __yield_context_handler<_Executor, exception_ptr, _Values...>
{
  typedef __exception_ptr_executor<_Executor> executor_type;
  typedef __yield_context_result<_Values...> _Result;

  __yield_context_handler(basic_yield_context<_Executor> __c)
    : _M_executor(__c._M_executor), _M_callee(__c._M_callee.lock()),
      _M_caller(__c._M_caller), _M_result(nullptr)
  {
  }

  executor_type get_executor() const noexcept
  {
    return executor_type(&_M_result->_M_exception, _M_executor);
  }

  template <class... _Args> void operator()(const exception_ptr& __e, _Args&&... __args)
  {
    _M_result->_M_exception = __e;
    _M_result->_Set_value(forward<_Args>(__args)...);
    if (_M_result->_M_ready.test_and_set())
    {
      typename __yield_context_callee::_Lock_guard __lock(*_M_callee);
      _M_callee->_M_coro();
    }
  }

  _Executor _M_executor;
  shared_ptr<__yield_context_callee> _M_callee;
  __yield_context_caller* _M_caller;
  __yield_context_result<_Values...>* _M_result;
};

template <class _Executor, class... _Values>
class async_result<__yield_context_handler<_Executor, _Values...>>
{
public:
  typedef __yield_context_handler<_Executor, _Values...> _Handler;
  typedef typename _Handler::_Result _Result;
  typedef decltype(declval<_Result>()._Get()) type;

  async_result(_Handler& __h)
    : _M_caller(__h._M_caller)
  {
    __h._M_result = &_M_result;
  }

  async_result(const async_result&) = delete;
  async_result& operator=(const async_result&) = delete;

  type get()
  {
    if (!_M_result._M_ready.test_and_set())
      (_M_caller->_M_coro)();
    return _M_result._Get();
  }

private:
  __yield_context_caller* _M_caller;
  _Result _M_result;
};

template <class _Executor, class _R, class... _Args>
struct handler_type<basic_yield_context<_Executor>, _R(_Args...)>
{
  typedef __yield_context_handler<_Executor, typename decay<_Args>::type...> type;
};

template <class _Executor, class _Func, class _Continuation, class... _Args>
struct __yield_context_entry_point
{
  typedef _Executor executor_type;

  _Func _M_func;
  _Continuation _M_continuation;
  tuple<_Args...> _M_args;
  executor_work<_Executor> _M_work;
  weak_ptr<__yield_context_callee> _M_callee;

  executor_type get_executor() const noexcept
  {
    return _M_work.get_executor();
  }

#ifndef EXECUTORS_NO_BOOST
  void operator()(boost::coroutines::pull_coroutine<void>& __coro)
  {
    if (_M_callee.expired()) // Workaround for bug in Boost.Coroutine in Boost 1.55.
      return;

    __yield_context_caller __caller{__coro};
    executor_work<_Executor> __w(std::move(_M_work));
    basic_yield_context<_Executor> __ctx(__w.get_executor());
    __ctx._M_callee = std::move(_M_callee);
    __ctx._M_caller = &__caller;

    typedef decltype(_Tuple_invoke(_M_func, _M_args, __ctx)) _Result;
    this->_Invoke(is_same<_Result, void>(), __ctx);
  }
#endif

  void _Invoke(true_type, const basic_yield_context<_Executor>& __ctx)
  {
    _Func __f(std::move(_M_func));
    _Continuation __c(std::move(_M_continuation));
    tuple<_Args...> __args(std::move(_M_args));
    _Tuple_invoke(__f, __args, __ctx);
    std::move(__c)();
  }

  void _Invoke(false_type, const basic_yield_context<_Executor>& __ctx)
  {
    _Func __f(std::move(_M_func));
    _Continuation __c(std::move(_M_continuation));
    tuple<_Args...> __args(std::move(_M_args));
    std::move(__c)(_Tuple_invoke(__f, __args, __ctx));
  }
};

struct __yield_null_continuation
{
  template <class... _T> void operator()(_T&&...) {}
};

template <class _Executor, class _Func, class _Continuation = __yield_null_continuation>
struct __yield_context_launcher
{
  typedef _Executor executor_type;

  _Func _M_func;
  _Continuation _M_continuation;
  executor_work<_Executor> _M_work;

  template <class _F> __yield_context_launcher(_F&& __f)
    : _M_func(forward<_F>(__f)), _M_work(associated_executor<_Func>::get(_M_func))
  {
  }

  template <class _F> __yield_context_launcher(
    executor_arg_t, const _Executor& __e, _F&& __f)
      : _M_func(forward<_F>(__f)), _M_work(__e)
  {
  }

  template <class _F, class _C> __yield_context_launcher(
    true_type, const executor_work<_Executor>& __w, _F&& __f, _C&& __c)
      : _M_func(forward<_F>(__f)), _M_continuation(forward<_C>(__c)), _M_work(__w)
  {
  }

  executor_type get_executor() const noexcept
  {
    return _M_work.get_executor();
  }

  template <class... _Args> void operator()(_Args&&... __args)
  {
    auto __callee = make_shared<__yield_context_callee>();

    __yield_context_entry_point<_Executor, _Func, _Continuation, typename decay<_Args>::type...>
      __ep{std::move(_M_func), std::move(_M_continuation),
        std::tie(forward<_Args>(__args)...), std::move(_M_work), __callee};

#ifdef EXECUTORS_NO_BOOST
    assert(0 && "boost required to make use of yield_context");
#else
    __callee->_M_coro = boost::coroutines::push_coroutine<void>(std::move(__ep));
#endif

    typename __yield_context_callee::_Lock_guard __lock(*__callee);
    __callee->_M_coro();
  }
};

template <class _ExecutorTo, class _Func, class _ExecutorFrom>
struct uses_executor<__yield_context_launcher<_ExecutorTo, _Func>, _ExecutorFrom>
  : is_convertible<_ExecutorFrom, _ExecutorTo> {};

template <class _T, class = void>
struct __is_yieldable : false_type {};

template <class _T>
struct __is_yieldable<_T,
  typename enable_if<is_convertible<__last_argument_t<__signature_t<_T>>,
    yield_context>::value>::type> : true_type {};

template <class _Func, class _R, class... _Args>
struct handler_type<_Func, _R(_Args...),
  typename enable_if<__is_yieldable<typename decay<_Func>::type>::value
    && !__is_executor_wrapper<typename decay<_Func>::type>::value>::type>
{
  typedef typename decay<_Func>::type _DecayFunc;
  typedef __last_argument_t<__signature_t<_DecayFunc>> _YieldContext;
  typedef associated_executor_t<_YieldContext> _Executor;
  typedef __yield_context_launcher<_Executor, _DecayFunc> type;
};

template <class _Executor, class _Func, class... _Args>
struct continuation_of<__yield_context_launcher<_Executor, _Func>(_Args...)>
{
  typedef typename decay<_Func>::type _DecayFunc;
  typedef __result_t<__signature_t<_DecayFunc>> _Result;
  typedef __make_signature_t<void, _Result> signature;

  template <class _F, class _Continuation>
  static auto chain(_F&& __f, _Continuation&& __c)
  {
    return __yield_context_launcher<_Executor, _Func,
      typename decay<_Continuation>::type>(true_type(), std::move(__f._M_work),
        std::move(__f._M_func), forward<_Continuation>(__c));
  }
};

} // inline namespace concurrency_v1
} // namespace experimental
} // namespace std

#endif
