//
// executor_wrapper.h
// ~~~~~~~~~~~~~~~~~~
// Wraps a function object so that it is associated with an executor.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_EXECUTOR_WRAPPER_H
#define EXECUTORS_EXPERIMENTAL_BITS_EXECUTOR_WRAPPER_H

#include <experimental/type_traits>

namespace std {
namespace experimental {

template <class _T, class _Executor>
class executor_wrapper
{
public:
  template <class _U> executor_wrapper(executor_arg_t, const _Executor& __e, _U&& __u)
    : executor_wrapper(__e, forward<_U>(__u))
  {
  }

  template <class _U> executor_wrapper(const executor_wrapper<_U, _Executor>& __w)
    : executor_wrapper(__w._M_executor, __w._M_wrapped)
  {
  }

  template <class _U> executor_wrapper(executor_arg_t, const _Executor& __e,
    const executor_wrapper<_U, _Executor>& __w)
      : executor_wrapper(__e, __w._M_wrapped)
  {
  }

  template <class _U> executor_wrapper(executor_wrapper<_U, _Executor>&& __w)
    : executor_wrapper(std::move(__w._M_executor), std::move(__w._M_wrapped))
  {
  }

  template <class _U> executor_wrapper(executor_arg_t, const _Executor& __e,
    executor_wrapper<_U, _Executor>&& __w)
      : executor_wrapper(__e, std::move(__w._M_wrapped))
  {
  }

  template <class... _Args> auto operator()(_Args&&... __args)
  {
    return _M_wrapped(forward<_Args>(__args)...);
  }

  template <class... _Args> auto operator()(_Args&&... __args) const
  {
    return _M_wrapped(forward<_Args>(__args)...);
  }

  friend _Executor get_executor(const executor_wrapper& __w)
  {
    return __w._M_executor;
  }

private:
  template <class _E, class _U> explicit executor_wrapper(_E&& __e, _U&& __u)
    : executor_wrapper(forward<_E>(__e), forward<_U>(__u), uses_executor<_T, _Executor>())
  {
  }

  template <class _E, class _U> explicit executor_wrapper(_E&& __e, _U&& __u, true_type)
    : _M_executor(forward<_E>(__e)), _M_wrapped(executor_arg, _M_executor, forward<_U>(__u))
  {
  }

  template <class _E, class _U> explicit executor_wrapper(_E&& __e, _U&& __u, false_type)
    : _M_executor(forward<_E>(__e)), _M_wrapped(forward<_U>(__u))
  {
  }

  template <class _U, class _E> friend class executor_wrapper;
  friend class async_result<executor_wrapper>;
  _Executor _M_executor;
  _T _M_wrapped;
};

template <class _T, class _Executor, class _Signature>
struct handler_type<executor_wrapper<_T, _Executor>, _Signature>
{
  typedef executor_wrapper<handler_type_t<_T, _Signature>, _Executor> type;
};

template <class _T, class _Executor>
class async_result<executor_wrapper<_T, _Executor>>
{
public:
  typedef typename async_result<_T>::type type;
  explicit async_result(executor_wrapper<_T, _Executor>& __w) : _M_wrapped(__w._M_wrapped) {}
  type get() { return _M_wrapped.get(); }

private:
  async_result<_T> _M_wrapped;
};

template <class _T, class _Executor>
inline auto wrap_with_executor(_T&& __t, const _Executor& __e)
{
  typedef typename decay<_T>::type _DecayT;
  typedef typename conditional<uses_executor<_DecayT, _Executor>::value,
    _DecayT, executor_wrapper<_DecayT, _Executor>>::type _Wrapper;
  return _Wrapper(executor_arg, __e, forward<_T>(__t));
}

} // namespace experimental
} // namespace std

#endif
