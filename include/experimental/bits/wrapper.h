//
// wrapper.h
// ~~~~~~~~~
// Wraps a function object so that it is associated with an executor.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_WRAPPER_H
#define EXECUTORS_EXPERIMENTAL_BITS_WRAPPER_H

#include <experimental/type_traits>

namespace std {
namespace experimental {

template <class _T, class _Executor>
class __wrapper
{
public:
  template <class _U> __wrapper(_U&& __u, const _Executor& __e)
    : _M_wrapped(forward<_U>(__u)), _M_executor(__e)
  {
  }

  template <class _U> __wrapper(const __wrapper<_U, _Executor>& __w)
    : _M_wrapped(__w._M_wrapped), _M_executor(__w._M_executor)
  {
  }

  template <class _U> __wrapper(__wrapper<_U, _Executor>&& __w)
    : _M_wrapped(std::move(__w._M_wrapped)), _M_executor(std::move(__w._M_executor))
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

  friend _Executor get_executor(const __wrapper& __w)
  {
    return __w._M_executor;
  }

private:
  template <class _U, class _E> friend class __wrapper;
  friend class async_result<__wrapper>;
  _T _M_wrapped;
  _Executor _M_executor;
};

template <class _CompletionToken, class _Executor, class _Signature>
struct handler_type<__wrapper<_CompletionToken, _Executor>, _Signature>
{
  typedef __wrapper<handler_type_t<_CompletionToken, _Signature>, _Executor> type;
};

template <class _Handler, class _Executor>
class async_result<__wrapper<_Handler, _Executor>>
{
public:
  typedef typename async_result<_Handler>::type type;
  explicit async_result(__wrapper<_Handler, _Executor>& __w) : _M_wrapped(__w._M_wrapped) {}
  type get() { return _M_wrapped.get(); }

private:
  async_result<_Handler> _M_wrapped;
};

} // namespace experimental
} // namespace std

#endif
