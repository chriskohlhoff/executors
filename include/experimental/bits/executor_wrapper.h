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
#include <experimental/bits/function_traits.h>

namespace std {
namespace experimental {

template <class _Executor>
class __executor_wrapper_executor
{
protected:
  template <class... _Args> explicit __executor_wrapper_executor(_Args&&... __args)
    : _M_executor(forward<_Args>(__args)...)
  {
  }

  _Executor _M_executor;
};

template <class _T, class = void>
class __executor_wrapper_wrapped;

template <class _R, class... _Args>
class __executor_wrapper_wrapped<_R (*)(_Args...)>
{
public:
  _R operator()(_Args... __args) const
  {
    return _M_wrapped(forward<_Args>(__args)...);
  }

protected:
  typedef _R (*_WrappedType)(_Args... __args);

  explicit __executor_wrapper_wrapped(_WrappedType __f)
    : _M_wrapped(__f)
  {
  }

  _WrappedType& _Wrapped() { return _M_wrapped; }
  const _WrappedType& _Wrapped() const { return _M_wrapped; }

private:
  _R (*_M_wrapped)(_Args...);
};

template <class _R, class... _Args>
class __executor_wrapper_wrapped<_R (&)(_Args...)>
{
public:
  _R operator()(_Args... __args) const
  {
    return _M_wrapped(forward<_Args>(__args)...);
  }

protected:
  typedef _R (&_WrappedType)(_Args... __args);

  explicit __executor_wrapper_wrapped(_WrappedType __f)
    : _M_wrapped(__f)
  {
  }

  _WrappedType& _Wrapped() { return _M_wrapped; }
  const _WrappedType& _Wrapped() const { return _M_wrapped; }

private:
  _R (&_M_wrapped)(_Args...);
};

template <class _T>
class __executor_wrapper_wrapped<_T,
  typename enable_if<is_class<_T>::value && __is_callable<_T>::value>::type>
    : private _T
{
public:
  using _T::operator();

protected:
  template <class... _Args> explicit __executor_wrapper_wrapped(_Args&&... __args)
    : _T(forward<_Args>(__args)...)
  {
  }

  _T& _Wrapped() { return *this; }
  const _T& _Wrapped() const { return *this; }
};

template <class _T>
class __executor_wrapper_wrapped<_T,
  typename enable_if<is_class<_T>::value && !__is_callable<_T>::value>::type>
    : private _T
{
protected:
  template <class... _Args> explicit __executor_wrapper_wrapped(_Args&&... __args)
    : _T(forward<_Args>(__args)...)
  {
  }

  _T& _Wrapped() { return *this; }
  const _T& _Wrapped() const { return *this; }
};

template <class _T>
class __executor_wrapper_wrapped<_T,
  typename enable_if<!is_class<_T>::value && __is_callable<_T>::value
    && !__is_callable_function<_T>::value>::type>
{
public:
  template <class... _Args> auto operator()(_Args&&... __args)
  {
    return _M_wrapped(forward<_Args>(__args)...);
  }

  template <class... _Args> auto operator()(_Args&&... __args) const
  {
    return _M_wrappedf(forward<_Args>(__args)...);
  }

protected:
  template <class _U, class _E> friend class executor_wrapper;

  template <class... _Args> explicit __executor_wrapper_wrapped(_Args&&... __args)
    : _M_wrapped(forward<_Args>(__args)...)
  {
  }

  _T& _Wrapped() { return _M_wrapped; }
  const _T& _Wrapped() const { return _M_wrapped; }

private:
  _T _M_wrapped;
};

template <class _T>
class __executor_wrapper_wrapped<_T,
  typename enable_if<!is_class<_T>::value && !__is_callable<_T>::value>::type>
{
protected:
  template <class _U, class _E> friend class executor_wrapper;

  template <class... _Args> explicit __executor_wrapper_wrapped(_Args&&... __args)
    : _M_wrapped(forward<_Args>(__args)...)
  {
  }

  _T& _Wrapped() { return _M_wrapped; }
  const _T& _Wrapped() const { return _M_wrapped; }

private:
  _T _M_wrapped;
};

template <class _T, class _Executor>
class executor_wrapper
  : public __executor_wrapper_executor<_Executor>,
    public __executor_wrapper_wrapped<_T>
{
public:
  template <class _U> executor_wrapper(executor_arg_t, const _Executor& __e, _U&& __u)
    : executor_wrapper(__e, forward<_U>(__u))
  {
  }

  template <class _U> executor_wrapper(const executor_wrapper<_U, _Executor>& __w)
    : executor_wrapper(__w._M_executor, __w._Wrapped())
  {
  }

  template <class _U> executor_wrapper(executor_arg_t, const _Executor& __e,
    const executor_wrapper<_U, _Executor>& __w)
      : executor_wrapper(__e, __w._Wrapped())
  {
  }

  template <class _U> executor_wrapper(executor_wrapper<_U, _Executor>&& __w)
    : executor_wrapper(std::move(__w._M_executor), std::move(__w._Wrapped()))
  {
  }

  template <class _U> executor_wrapper(executor_arg_t, const _Executor& __e,
    executor_wrapper<_U, _Executor>&& __w)
      : executor_wrapper(__e, std::move(__w._Wrapped()))
  {
  }

  friend _Executor make_executor(const executor_wrapper& __w)
  {
    return __w._M_executor;
  }

private:
  template <class _E, class _U> explicit executor_wrapper(_E&& __e, _U&& __u)
    : executor_wrapper(forward<_E>(__e), forward<_U>(__u), uses_executor<_T, _Executor>())
  {
  }

  template <class _E, class _U> explicit executor_wrapper(_E&& __e, _U&& __u, true_type)
    : __executor_wrapper_executor<_Executor>(forward<_E>(__e)),
      __executor_wrapper_wrapped<_T>(executor_arg, this->_M_executor, forward<_U>(__u))
  {
  }

  template <class _E, class _U> explicit executor_wrapper(_E&& __e, _U&& __u, false_type)
    : __executor_wrapper_executor<_Executor>(forward<_E>(__e)),
      __executor_wrapper_wrapped<_T>(forward<_U>(__u))
  {
  }

  template <class _U, class _E> friend class executor_wrapper;
  friend class async_result<executor_wrapper>;
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
  explicit async_result(executor_wrapper<_T, _Executor>& __w) : _M_wrapped(__w._Wrapped()) {}
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

template <class _T>
struct __is_executor_wrapper : false_type {};

template <class _T, class _Executor>
struct __is_executor_wrapper<executor_wrapper<_T, _Executor>> : true_type {};

} // namespace experimental
} // namespace std

#endif
