//
// use_future.h
// ~~~~~~~~~~~~
// A completion token used so that asynchronous operations return futures.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_USE_FUTURE_H
#define EXECUTORS_EXPERIMENTAL_BITS_USE_FUTURE_H

#include <exception>
#include <memory>
#include <system_error>
#include <tuple>
#include <utility>
#include <experimental/executor>

namespace std {
namespace experimental {
inline namespace concurrency_v2 {

template <class _T, class _Func, class... _Args>
inline void _Promise_invoke(promise<_T>& __p, _Func& __f, _Args&&... __args)
{
  try
  {
    return __f(forward<_Args>(__args)...);
  }
  catch (...)
  {
    __p.set_exception(current_exception());
  }
}

template <class _T, class _Func, class... _Args>
inline void _Promise_invoke_and_set(promise<_T>& __p, _Func& __f, _Args&&... __args)
{
  try
  {
    __p.set_value(__f(forward<_Args>(__args)...));
  }
  catch (...)
  {
    __p.set_exception(current_exception());
  }
}

template <class _Func, class... _Args>
inline void _Promise_invoke_and_set(promise<void>& __p, _Func& __f, _Args&&... __args)
{
  try
  {
    __f(forward<_Args>(__args)...);
    __p.set_value();
  }
  catch (...)
  {
    __p.set_exception(current_exception());
  }
}

template <class _T, class _Func>
struct __promise_invoker
{
  shared_ptr<promise<_T>> _M_promise;
  typename decay<_Func>::type _M_func;

  void operator()() { _Promise_invoke(*_M_promise, _M_func); }
};

template <class _T>
struct __promise_executor
{
  shared_ptr<promise<_T>> _M_promise;

  execution_context& context() noexcept { return system_executor().context(); }

  void on_work_started() noexcept {}
  void on_work_finished() noexcept {}

  template <class _F, class _A> void dispatch(_F&& __f, const _A&)
    { __promise_invoker<_T, _F>{_M_promise, forward<_F>(__f)}(); }
  template <class _F, class _A> void post(_F&& __f, const _A& __a)
    { system_executor().post(__promise_invoker<_T, _F>{_M_promise, forward<_F>(__f)}, __a); }
  template <class _F, class _A> void defer(_F&& __f, const _A& __a)
    { system_executor().defer(__promise_invoker<_T, _F>{_M_promise, forward<_F>(__f)}, __a); }

  friend bool operator==(const __promise_executor& __a, const __promise_executor& __b) noexcept
    { return __a._M_promise == __b._M_promise; }
  friend bool operator!=(const __promise_executor& __a, const __promise_executor& __b) noexcept
    { return __a._M_promise != __b._M_promise; }
};

template <class _T>
struct __promise_creator
{
  typedef future<_T> _Future;
  shared_ptr<promise<_T>> _M_promise;

  template <class _Alloc>
    __promise_creator(const _Alloc& __a)
      : _M_promise(allocate_shared<promise<_T>>(
        typename _Alloc::template rebind<char>::other(__a), allocator_arg,
          typename _Alloc::template rebind<char>::other(__a))) {}

  typedef __promise_executor<_T> executor_type;
  executor_type get_executor() const noexcept { return { _M_promise }; }
};

template <class... _Args>
struct __promise_setter : __promise_creator<tuple<typename decay<_Args>::type...>>
{
  using __promise_creator<tuple<typename decay<_Args>::type...>>::__promise_creator;

  void _Set(_Args... __args)
    { this->_M_promise->set_value(std::forward_as_tuple(std::forward<_Args>(__args)...)); }
};

template <class _Arg>
struct __promise_setter<_Arg> : __promise_creator<typename decay<_Arg>::type>
{
  using __promise_creator<typename decay<_Arg>::type>::__promise_creator;

  void _Set(_Arg __arg) { this->_M_promise->set_value(forward<_Arg>(__arg)); }
};

template <>
struct __promise_setter<> : __promise_creator<void>
{
  using __promise_creator<void>::__promise_creator;

  void _Set() { this->_M_promise->set_value(); }
};

template <class... _Args>
struct __promise_handler : __promise_setter<_Args...>
{
  template <class _Alloc>
  __promise_handler(use_future_t<_Alloc> __u)
    : __promise_setter<_Args...>(__u.get_allocator()) {}

  void operator()(_Args... __args) { this->_Set(forward<_Args>(__args)...); }
};

template <class... _Args>
struct __promise_handler<error_code, _Args...> : __promise_setter<_Args...>
{
  template <class _Alloc>
  __promise_handler(use_future_t<_Alloc> __u)
    : __promise_setter<_Args...>(__u.get_allocator()) {}

  void operator()(const error_code& __e, _Args... __args)
  {
    if (__e)
      this->_M_promise->set_exception(make_exception_ptr(system_error(__e)));
    else
      this->_Set(forward<_Args>(__args)...);
  }
};

template <class... _Args>
struct __promise_handler<exception_ptr, _Args...> : __promise_setter<_Args...>
{
  template <class _Alloc>
  __promise_handler(use_future_t<_Alloc> __u)
    : __promise_setter<_Args...>(__u.get_allocator()) {}

  void operator()(const exception_ptr& __e, _Args... __args)
  {
    if (__e)
      this->_M_promise->set_exception(__e);
    else
      this->_Set(forward<_Args>(__args)...);
  }
};

template <class _Alloc, class _R, class... _Args>
class async_result<use_future_t<_Alloc>, _R(_Args...)>
{
public:
  typedef __promise_handler<_Args...> completion_handler_type;
  typedef typename completion_handler_type::_Future return_type;

  async_result(completion_handler_type& __h) : _M_future(__h._M_promise->get_future()) {}
  async_result(const async_result&) = delete;
  async_result& operator=(const async_result&) = delete;

  return_type get() { return std::move(_M_future); }

private:
  return_type _M_future;
};

template <class _Func, class _Alloc>
struct __packaged_token
{
  _Func _M_func;
  _Alloc _M_allocator;
};

template <class _Func, class _Alloc, class... _Args>
struct __packaged_handler : __promise_creator<typename result_of<_Func(_Args...)>::type>
{
  _Func _M_func;
  _Alloc _M_allocator;

  __packaged_handler(__packaged_token<_Func, _Alloc>&& __token)
    : __promise_creator<typename result_of<_Func(_Args...)>::type>(__token._M_allocator),
      _M_func(std::move(__token._M_func)), _M_allocator(__token._M_allocator) {}

  typedef _Alloc allocator_type;
  allocator_type get_allocator() const noexcept { return _M_allocator; }

  void operator()(_Args... __args)
    { _Promise_invoke_and_set(*this->_M_promise, _M_func, forward<_Args>(__args)...); }
};

template <class _Func, class _Alloc, class _R, class... _Args>
class async_result<__packaged_token<_Func, _Alloc>, _R(_Args...)>
{
public:
  typedef __packaged_handler<_Func, _Alloc, _Args...> completion_handler_type;
  typedef typename completion_handler_type::_Future return_type;

  async_result(completion_handler_type& __h) : _M_future(__h._M_promise->get_future()) {}
  async_result(const async_result&) = delete;
  async_result& operator=(const async_result&) = delete;

  return_type get() { return std::move(_M_future); }

private:
  return_type _M_future;
};

template <class _Alloc> template <class _F>
inline __packaged_token<typename decay<_F>::type, _Alloc>
use_future_t<_Alloc>::operator()(_F&& __f) const
  { return { std::forward<_F>(__f), _M_allocator }; }

} // inline namespace concurrency_v2
} // namespace experimental
} // namespace std

#endif
