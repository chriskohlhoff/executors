//
// promise_handler.h
// ~~~~~~~~~~~~~~~~~
// A function object adapter to allow a promise to be used in a callback.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_PROMISE_HANDLER_H
#define EXECUTORS_EXPERIMENTAL_BITS_PROMISE_HANDLER_H

#include <exception>
#include <memory>
#include <system_error>
#include <tuple>
#include <utility>
#include <experimental/executor>

namespace std {
namespace experimental {
inline namespace concurrency_v1 {

template <class _Func, class _Promise>
struct __promise_invoker
{
  shared_ptr<_Promise> _M_promise;
  _Func _M_func;

  template <class _F>
  __promise_invoker(const shared_ptr<_Promise>& __p, _F&& __f)
    : _M_promise(__p), _M_func(forward<_F>(__f)) {}

  void operator()()
  {
    try
    {
      std::move(_M_func)();
    }
    catch (...)
    {
      _M_promise->set_exception(current_exception());
    }
  }
};

template <class _Promise>
struct __promise_executor
{
  shared_ptr<_Promise> _M_promise;

  execution_context& context() noexcept
  {
    return system_executor().context();
  }

  void on_work_started() noexcept
  {
  }

  void on_work_finished() noexcept
  {
  }

  template <class _F, class _A> void dispatch(_F&& __f, const _A&)
  {
    typedef typename decay<_F>::type _Func;
    __promise_invoker<_Func, _Promise>(_M_promise, forward<_F>(__f))();
  }

  template <class _F, class _A> void post(_F&& __f, const _A& __a)
  {
    typedef typename decay<_F>::type _Func;
    system_executor().post(
      __promise_invoker<_Func, _Promise>(_M_promise, forward<_F>(__f)), __a);
  }

  template <class _F, class _A> void defer(_F&& __f, const _A& __a)
  {
    typedef typename decay<_F>::type _Func;
    system_executor().defer(
      __promise_invoker<_Func, _Promise>(_M_promise, forward<_F>(__f)), __a);
  }

  friend bool operator==(const __promise_executor& __a, const __promise_executor& __b) noexcept
  {
    return __a._M_promise == __b._M_promise;
  }

  friend bool operator!=(const __promise_executor& __a, const __promise_executor& __b) noexcept
  {
    return __a._M_promise != __b._M_promise;
  }
};

template <class _Promise>
struct is_executor<__promise_executor<_Promise>> : true_type {};

template <class... _Args>
struct __value_pack
{
  typedef tuple<typename decay<_Args>::type...> _Type;

  static void _Apply(promise<_Type>& __p, _Args... __args)
  {
    __p.set_value(std::make_tuple(forward<_Args>(__args)...));
  }
};

template <class _Arg>
struct __value_pack<_Arg>
{
  typedef typename decay<_Arg>::type _Type;

  static void _Apply(promise<_Type>& __p, _Arg __arg)
  {
    __p.set_value(forward<_Arg>(__arg));
  }
};

template <>
struct __value_pack<>
{
  typedef void _Type;

  static void _Apply(promise<_Type>& __p)
  {
    __p.set_value();
  }
};

template <class... _Args>
struct __promise_handler
{
  typedef promise<typename __value_pack<_Args...>::_Type> _Promise;
  typedef __promise_executor<_Promise> executor_type;
  shared_ptr<_Promise> _M_promise;

  template <class _Alloc>
  __promise_handler(use_future_t<_Alloc> __u)
    : _M_promise(make_shared<_Promise>(allocator_arg, __u.get_allocator())) {}

  executor_type get_executor() const noexcept
  {
    return __promise_executor<_Promise>{_M_promise};
  }

  void operator()(_Args... __args)
  {
    __value_pack<_Args...>::_Apply(*_M_promise, forward<_Args>(__args)...);
  }
};

template <class... _Args>
struct __promise_handler<error_code, _Args...>
{
  typedef promise<typename __value_pack<_Args...>::_Type> _Promise;
  typedef __promise_executor<_Promise> executor_type;
  shared_ptr<_Promise> _M_promise;

  template <class _Alloc>
  __promise_handler(use_future_t<_Alloc> __u)
    : _M_promise(make_shared<_Promise>(allocator_arg, __u.get_allocator())) {}

  executor_type get_executor() const noexcept
  {
    return __promise_executor<_Promise>{_M_promise};
  }

  void operator()(const error_code& __e, _Args... __args)
  {
    if (__e)
      _M_promise->set_exception(make_exception_ptr(system_error(__e)));
    else
      __value_pack<_Args...>::_Apply(*_M_promise, forward<_Args>(__args)...);
  }
};

template <class... _Args>
struct __promise_handler<exception_ptr, _Args...>
{
  typedef promise<typename __value_pack<_Args...>::_Type> _Promise;
  typedef __promise_executor<_Promise> executor_type;
  shared_ptr<_Promise> _M_promise;

  template <class _Alloc>
  __promise_handler(use_future_t<_Alloc> __u)
    : _M_promise(make_shared<_Promise>(allocator_arg, __u.get_allocator())) {}

  executor_type get_executor() const noexcept
  {
    return __promise_executor<_Promise>{_M_promise};
  }

  void operator()(const exception_ptr& __e, _Args... __args)
  {
    if (__e)
      _M_promise->set_exception(__e);
    else
      __value_pack<_Args...>::_Apply(*_M_promise, forward<_Args>(__args)...);
  }
};

template <class... _Args>
class async_result<__promise_handler<_Args...>>
{
public:
  typedef __promise_handler<_Args...> _Handler;
  typedef decltype(*declval<_Handler>()._M_promise) _Promise;
  typedef decltype(declval<_Promise>().get_future()) type;

  async_result(_Handler& __h)
    : _M_future(__h._M_promise->get_future()) {}
  async_result(const async_result&) = delete;
  async_result& operator=(const async_result&) = delete;

  type get() { return std::move(_M_future); }

private:
  type _M_future;
};

template <class _Alloc, class _R, class... _Args>
struct handler_type<use_future_t<_Alloc>, _R(_Args...)>
{
  typedef __promise_handler<_Args...> type;
};

} // inline namespace concurrency_v1
} // namespace experimental
} // namespace std

#endif
