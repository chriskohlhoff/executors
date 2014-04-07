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

template <class... _Values>
struct __value_pack
{
  typedef tuple<_Values...> _Type;

  template <class... _Args>
  static void _Apply(promise<_Type>& __p, _Args&&... __args)
  {
    __p.set_value(std::make_tuple(forward<_Args>(__args)...));
  }
};

template <class _Value>
struct __value_pack<_Value>
{
  typedef _Value _Type;

  template <class _Arg>
  static void _Apply(promise<_Type>& __p, _Arg&& __arg)
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

template <class... _Values>
struct __promise_handler
{
  typedef promise<typename __value_pack<_Values...>::_Type> _Promise;
  shared_ptr<_Promise> _M_promise;

  template <class _Alloc>
  __promise_handler(use_future_t<_Alloc> __u)
    : _M_promise(make_shared<_Promise>(allocator_arg, __u.get_allocator())) {}

  template <class... _Args>
  void operator()(_Args&&... __args)
  {
    __value_pack<_Values...>::_Apply(*_M_promise, forward<_Args>(__args)...);
  }
};

template <class... _Values>
struct __promise_handler<error_code, _Values...>
{
  typedef promise<typename __value_pack<_Values...>::_Type> _Promise;
  shared_ptr<_Promise> _M_promise;

  template <class _Alloc>
  __promise_handler(use_future_t<_Alloc> __u)
    : _M_promise(make_shared<_Promise>(allocator_arg, __u.get_allocator())) {}

  template <class... _Args>
  void operator()(const error_code& __e, _Args&&... __args)
  {
    if (__e)
      _M_promise->set_exception(make_exception_ptr(system_error(__e)));
    else
      __value_pack<_Values...>::_Apply(*_M_promise, forward<_Args>(__args)...);
  }
};

template <class... _Values>
struct __promise_handler<exception_ptr, _Values...>
{
  typedef promise<typename __value_pack<_Values...>::_Type> _Promise;
  shared_ptr<_Promise> _M_promise;

  template <class _Alloc>
  __promise_handler(use_future_t<_Alloc> __u)
    : _M_promise(make_shared<_Promise>(allocator_arg, __u.get_allocator())) {}

  template <class... _Args>
  void operator()(const exception_ptr& __e, _Args&&... __args)
  {
    if (__e)
      _M_promise->set_exception(__e);
    else
      __value_pack<_Values...>::_Apply(*_M_promise, forward<_Args>(__args)...);
  }
};

template <class _Func, class... _Values>
struct __promise_invoker
{
  typedef typename __promise_handler<_Values...>::_Promise _Promise;
  shared_ptr<_Promise> _M_promise;
  _Func _M_func;

  template <class _F>
  __promise_invoker(const shared_ptr<_Promise>& __p, _F&& __f)
    : _M_promise(__p), _M_func(forward<_F>(__f)) {}

  void operator()() &&
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

template <class... _Values>
struct __promise_executor
{
  typedef typename __promise_handler<_Values...>::_Promise _Promise;
  shared_ptr<_Promise> _M_promise;

  struct work
  {
    shared_ptr<_Promise> _M_promise;

    friend __promise_executor make_executor(const work& __w)
    {
      return __promise_executor{__w._M_promise};
    }
  };

  work make_work() { return work{_M_promise}; }

  template <class _F> void post(_F&& __f)
  {
    typedef typename decay<_F>::type _Func;
    system_executor().post(
      __promise_invoker<_Func, _Values...>(_M_promise, forward<_F>(__f)));
  }

  template <class _F> void dispatch(_F&& __f)
  {
    typedef typename decay<_F>::type _Func;
    __promise_invoker<_Func, _Values...>(_M_promise, forward<_F>(__f))();
  }

  template <class _Func>
  inline auto wrap(_Func&& __f)
  {
    return (wrap_with_executor)(forward<_Func>(__f), *this);
  }

  execution_context& context()
  {
    return system_executor().context();
  }

  friend __promise_executor make_executor(const __promise_executor& __e)
  {
    return __e;
  }
};

template <class... _Values>
struct is_executor<__promise_executor<_Values...>> : true_type {};

template <class... _Values>
inline auto make_executor(const __promise_handler<_Values...>& __h)
{
  return __promise_executor<_Values...>{__h._M_promise};
}

template <class... _Values>
class async_result<__promise_handler<_Values...>>
{
public:
  typedef __promise_handler<_Values...> _Handler;
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
  typedef __promise_handler<typename decay<_Args>::type...> type;
};

} // namespace experimental
} // namespace std

#endif
