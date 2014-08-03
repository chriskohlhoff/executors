//
// await_context.h
// ~~~~~~~~~~~~~~~
// Stackless coroutine implementation.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_AWAIT_CONTEXT_H
#define EXECUTORS_EXPERIMENTAL_BITS_AWAIT_CONTEXT_H

#include <exception>
#include <system_error>
#include <tuple>
#include <utility>
#include <experimental/bits/exception_ptr_executor.h>
#include <experimental/bits/executor_wrapper.h>
#include <experimental/bits/function_traits.h>
#include <experimental/bits/invoker.h>
#include <experimental/bits/tuple_utils.h>

namespace std {
namespace experimental {
inline namespace concurrency_v1 {

template <class _Executor> template <class _OtherExecutor>
basic_await_context<_Executor>::basic_await_context(const basic_await_context<_OtherExecutor>& __c)
  : _M_executor(__c._M_executor), _M_impl(__c._M_impl), _M_error_code(__c._M_error_code)
{
}

template <class _Executor>
basic_await_context<_Executor> basic_await_context<_Executor>::operator[](error_code& __ec) const
{
  basic_await_context<_Executor> __c(*this);
  __c._M_error_code = &__ec;
  return __c;
}

template <class _Executor>
inline typename basic_await_context<_Executor>::executor_type
basic_await_context<_Executor>::get_executor() const noexcept
{
  return _M_executor;
}

template <class _T> class __awaitable {};

class __coroutine
{
public:
  __coroutine() : _M_value(0) {}
  bool _Is_complete() const { return _M_value == -1; }
  friend __coroutine& _Get_coroutine(__coroutine& __c) { return __c; }
  friend __coroutine& _Get_coroutine(__coroutine* __c) { return *__c; }
  friend exception_ptr* _Get_coroutine_exception(__coroutine&) { return 0; }
  friend exception_ptr* _Get_coroutine_exception(__coroutine*) { return 0; }
  friend void** _Get_coroutine_async_result(__coroutine&) { return 0; }
  friend void** _Get_coroutine_async_result(__coroutine*) { return 0; }

private:
  friend class __coroutine_ref;
  int _M_value;
};

class __coroutine_ref
{
public:
  __coroutine_ref(__coroutine& __c, exception_ptr* __ex, void** __r)
    : _M_value(__c._M_value), _M_modified(false), _M_ex(__ex), _M_async_result(__r)
  {
  }

  ~__coroutine_ref()
  {
    if (!_M_modified)
      _M_value = -1;
  }

  operator int() const
  {
    return _M_value;
  }

  int& operator=(int __v)
  {
    _M_modified = true;
    return _M_value = __v;
  }

  template <class _T>
  __awaitable<_T> operator&(_T& t)
  {
    *_M_async_result = &t;
    return __awaitable<_T>();
  }

  template <class _T> void operator&(__awaitable<_T>)
  {
  }

  void _Rethrow() const
  {
    if (_M_ex && *_M_ex)
      rethrow_exception(*_M_ex);
  }

private:
  __coroutine_ref& operator=(const __coroutine_ref&) = delete;
  int& _M_value;
  bool _M_modified;
  const exception_ptr* const _M_ex;
  void** const _M_async_result;
};

#define __REENTER(__c) \
  switch (::std::experimental::__coroutine_ref __coro_ref = \
      ::std::experimental::__coroutine_ref(_Get_coroutine(__c), \
        _Get_coroutine_exception(__c), _Get_coroutine_async_result(__c))) \
    case -1: if (__coro_ref) \
    { \
      goto __terminate_coroutine; \
      __terminate_coroutine: \
      __coro_ref = -1; \
      goto __bail_out_of_coroutine; \
      __bail_out_of_coroutine: \
      break; \
    } \
    else case 0:

#define __AWAIT(__n) \
  for (__coro_ref = (__n);;) \
    if (__coro_ref == 0) \
    { \
      case (__n): ; \
      __coro_ref._Rethrow(); \
      break; \
    } \
    else \
      switch (__coro_ref ? 0 : 1) \
        for (;;) \
          case -1: if (__coro_ref) \
            goto __terminate_coroutine; \
          else for (;;) \
            case 1: if (__coro_ref) \
              goto __bail_out_of_coroutine; \
            else case 0: __coro_ref&

#define reenter __REENTER
#define await __AWAIT(__LINE__)

class __await_context_impl_base
{
public:
  virtual ~__await_context_impl_base() {}
  virtual void _Resume() = 0;

  friend __coroutine& _Get_coroutine(__await_context_impl_base& __i) { return __i._M_coroutine; }
  friend __coroutine& _Get_coroutine(__await_context_impl_base* __i) { return __i->_M_coroutine; }
  friend exception_ptr* _Get_coroutine_exception(__await_context_impl_base& __i) { return &__i._M_ex; }
  friend exception_ptr* _Get_coroutine_exception(__await_context_impl_base* __i) { return &__i->_M_ex; }
  friend void** _Get_coroutine_async_result(__await_context_impl_base& __i) { return &__i._M_result_value; }
  friend void** _Get_coroutine_async_result(__await_context_impl_base* __i) { return &__i->_M_result_value; }

private:
  template <class, class...> friend struct __await_context_handler;
  template <class> friend class basic_await_context;
  __coroutine _M_coroutine;
  exception_ptr _M_ex = nullptr;
  error_code* _M_result_ec = nullptr;
  void* _M_result_value = nullptr;
};

template <class _Executor, class _Func, class _Continuation, class... _Args>
class __await_context_impl
  : public __await_context_impl_base,
    public enable_shared_from_this<__await_context_impl<_Executor, _Func, _Continuation, _Args...>>
{
public:
  template <class _F, class _C, class... _A>
  __await_context_impl(const executor_work<_Executor>& __w, _F&& __f, _C&& __c, _A&&... __args)
    : _M_work(__w), _M_function(forward<_F>(__f)), _M_invocations(0),
      _M_continuation(forward<_C>(__c)), _M_args(forward<_A>(__args)...)
  {
  }

  virtual void _Resume()
  {
    const basic_await_context<_Executor> __ctx(_M_work, this->shared_from_this());
    typedef decltype(_Tuple_invoke(_M_function, _M_args, __ctx)) _Result;
    this->_Invoke(is_same<_Result, void>(), __ctx);
  }

private:
  void _Invoke(true_type, const basic_await_context<_Executor>& __ctx)
  {
    if (++_M_invocations == 1)
    {
      for (;;)
      {
        _Tuple_invoke(_M_function, _M_args, __ctx);
        if (_Get_coroutine(__ctx)._Is_complete())
          std::move(_M_continuation)();
        else if (--_M_invocations != 0)
          continue;
        return;
      }
    }
  }

  void _Invoke(false_type, const basic_await_context<_Executor>& __ctx)
  {
    if (++_M_invocations == 1)
    {
      for (;;)
      {
        auto __r = _Tuple_invoke(_M_function, _M_args, __ctx);
        if (_Get_coroutine(__ctx)._Is_complete())
          std::move(_M_continuation)(std::move(__r));
        else if (--_M_invocations != 0)
          continue;
        return;
      }
    }
  }

  executor_work<_Executor> _M_work;
  _Func _M_function;
  atomic<size_t> _M_invocations;
  _Continuation _M_continuation;
  tuple<_Args...> _M_args;
};

template <class... _Values>
struct __await_context_result
{
  typedef decltype(_Tuple_get(declval<tuple<_Values...>>())) _Type;

  template <class... _Args>
  static void _Set_value(void* __r, _Args&&... __args)
  {
    *static_cast<_Type*>(__r) = _Tuple_get(std::make_tuple(forward<_Args>(__args)...));
  }
};

template <>
struct __await_context_result<>
{
  typedef void _Type;

  static void _Set_value(void*)
  {
  }
};

template <class _Executor, class... _Values>
struct __await_context_handler
{
  typedef __exception_ptr_executor<_Executor> executor_type;
  typedef __await_context_result<_Values...> _Result;

  __await_context_handler(basic_await_context<_Executor> __c)
    : _M_executor(__c._M_executor), _M_impl(__c._M_impl)
  {
  }

  executor_type get_executor() const noexcept
  {
    return executor_type(&_M_impl->_M_ex, _M_executor);
  }

  template <class... _Args> void operator()(_Args&&... __args)
  {
    _Result::_Set_value(_M_impl->_M_result_value, forward<_Args>(__args)...);
    _M_impl->_Resume();
  }

  _Executor _M_executor;
  shared_ptr<__await_context_impl_base> _M_impl;
};

template <class _Executor, class... _Values>
struct __await_context_handler<_Executor, error_code, _Values...>
{
  typedef __exception_ptr_executor<_Executor> executor_type;
  typedef __await_context_result<_Values...> _Result;

  __await_context_handler(basic_await_context<_Executor> __c)
    : _M_executor(__c._M_executor), _M_impl(__c._M_impl)
  {
  }

  executor_type get_executor() const noexcept
  {
    return executor_type(&_M_impl->_M_ex, _M_executor);
  }

  template <class... _Args> void operator()(const error_code& __e, _Args&&... __args)
  {
    if (_M_impl->_M_result_ec)
      *_M_impl->_M_result_ec = __e;
    else if (__e)
      _M_impl->_M_ex = make_exception_ptr(system_error(__e));
    _Result::_Set_value(_M_impl->_M_result_value, forward<_Args>(__args)...);
    _M_impl->_Resume();
  }

  _Executor _M_executor;
  shared_ptr<__await_context_impl_base> _M_impl;
};

template <class _Executor, class... _Values>
struct __await_context_handler<_Executor, exception_ptr, _Values...>
{
  typedef __exception_ptr_executor<_Executor> executor_type;
  typedef __await_context_result<_Values...> _Result;

  __await_context_handler(basic_await_context<_Executor> __c)
    : _M_executor(__c._M_executor), _M_impl(__c._M_impl)
  {
  }

  executor_type get_executor() const noexcept
  {
    return executor_type(&_M_impl->_M_ex, _M_executor);
  }

  template <class... _Args> void operator()(const exception_ptr& __e, _Args&&... __args)
  {
    if (__e)
      _M_impl->_M_ex = make_exception_ptr(__e);
    _Result::_Set_value(_M_impl->_M_result_value, forward<_Args>(__args)...);
    _M_impl->_Resume();
  }

  _Executor _M_executor;
  shared_ptr<__await_context_impl_base> _M_impl;
};

template <class _Executor, class... _Values>
class async_result<__await_context_handler<_Executor, _Values...>>
{
public:
  typedef __await_context_handler<_Executor, _Values...> _Handler;
  typedef typename _Handler::_Result _Result;
  typedef __awaitable<typename _Result::_Type> type;

  async_result(_Handler&)
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
struct handler_type<basic_await_context<_Executor>, _R(_Args...)>
{
  typedef __await_context_handler<_Executor, typename decay<_Args>::type...> type;
};

struct __await_null_continuation
{
  template <class... _T> void operator()(_T&&...) {}
};

template <class _Executor, class _Func, class _Continuation = __await_null_continuation>
struct __await_context_launcher
{
  typedef _Executor executor_type;

  executor_work<_Executor> _M_work;
  _Func _M_func;
  _Continuation _M_continuation;

  template <class _F> __await_context_launcher(_F&& __f)
    : _M_work(associated_executor<_Func>::get(__f)), _M_func(forward<_F>(__f))
  {
  }

  template <class _F> __await_context_launcher(
    executor_arg_t, const _Executor& __e, _F&& __f)
      : _M_work(__e), _M_func(forward<_F>(__f))
  {
  }

  template <class _F, class _C> __await_context_launcher(
    true_type, const executor_work<_Executor>& __w, _F&& __f, _C&& __c)
      : _M_work(__w), _M_func(forward<_F>(__f)), _M_continuation(forward<_C>(__c))
  {
  }

  executor_type get_executor() const noexcept
  {
    return _M_work.get_executor();
  }

  template <class... _Args> void operator()(_Args&&... __args)
  {
    auto __impl = make_shared<__await_context_impl<
      _Executor, _Func, _Continuation, typename decay<_Args>::type...>>(_M_work,
        std::move(_M_func), std::move(_M_continuation), forward<_Args>(__args)...);
    __impl->_Resume();
  }
};

template <class _ExecutorTo, class _Func, class _ExecutorFrom>
struct uses_executor<__await_context_launcher<_ExecutorTo, _Func>, _ExecutorFrom>
  : is_convertible<_ExecutorFrom, _ExecutorTo> {};

template <class _T, class = void>
struct __is_awaitable : false_type {};

template <class _T>
struct __is_awaitable<_T,
  typename enable_if<is_convertible<__last_argument_t<__signature_t<_T>>,
    await_context>::value>::type> : true_type {};

template <class _Func, class _R, class... _Args>
struct handler_type<_Func, _R(_Args...),
  typename enable_if<__is_awaitable<typename decay<_Func>::type>::value
    && !__is_executor_wrapper<typename decay<_Func>::type>::value>::type>
{
  typedef typename decay<_Func>::type _DecayFunc;
  typedef __last_argument_t<__signature_t<_DecayFunc>> _AwaitContext;
  typedef associated_executor_t<_AwaitContext> _Executor;
  typedef __await_context_launcher<_Executor, _DecayFunc> type;
};

template <class _Executor, class _Func, class... _Args>
struct continuation_of<__await_context_launcher<_Executor, _Func>(_Args...)>
{
  typedef typename decay<_Func>::type _DecayFunc;
  typedef __result_t<__signature_t<_DecayFunc>> _Result;
  typedef __make_signature_t<void, _Result> signature;

  template <class _F, class _Continuation>
  static auto chain(_F&& __f, _Continuation&& __c)
  {
    return __await_context_launcher<_Executor, _Func,
      typename decay<_Continuation>::type>(true_type(), std::move(__f._M_work),
        std::move(__f._M_func), forward<_Continuation>(__c));
  }
};

} // inline namespace concurrency_v1
} // namespace experimental
} // namespace std

#endif
