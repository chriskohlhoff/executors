//
// executor_wrapper_base.h
// ~~~~~~~~~~~~~~~~~~~~~~~
// Wraps a function object so that it is associated with an executor.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_EXECUTOR_WRAPPER_BASE_H
#define EXECUTORS_EXPERIMENTAL_BITS_EXECUTOR_WRAPPER_BASE_H

#include <experimental/type_traits>
#include <experimental/bits/function_traits.h>
#include <utility>

namespace std {
namespace experimental {

template <class _Executor>
class __executor_wrapper_base_executor
{
protected:
  template <class... _Args> explicit __executor_wrapper_base_executor(_Args&&... __args)
    : _M_executor(forward<_Args>(__args)...)
  {
  }

  _Executor _M_executor;
};

template <class _T, class = void>
class __executor_wrapper_base_wrapped;

template <class _R, class... _Args>
class __executor_wrapper_base_wrapped<_R (*)(_Args...)>
{
public:
  _R operator()(_Args... __args) const
  {
    return _M_wrapped(forward<_Args>(__args)...);
  }

protected:
  typedef _R (*_WrappedType)(_Args... __args);

  explicit __executor_wrapper_base_wrapped(_WrappedType __f)
    : _M_wrapped(__f)
  {
  }

  _WrappedType& _Wrapped() { return _M_wrapped; }
  const _WrappedType& _Wrapped() const { return _M_wrapped; }

private:
  _R (*_M_wrapped)(_Args...);
};

template <class _R, class... _Args>
class __executor_wrapper_base_wrapped<_R (&)(_Args...)>
{
public:
  _R operator()(_Args... __args) const
  {
    return _M_wrapped(forward<_Args>(__args)...);
  }

protected:
  typedef _R (&_WrappedType)(_Args... __args);

  explicit __executor_wrapper_base_wrapped(_WrappedType __f)
    : _M_wrapped(__f)
  {
  }

  _WrappedType& _Wrapped() { return _M_wrapped; }
  const _WrappedType& _Wrapped() const { return _M_wrapped; }

private:
  _R (&_M_wrapped)(_Args...);
};

template <class _T>
class __executor_wrapper_base_wrapped<_T,
  typename enable_if<is_class<_T>::value && __is_callable<_T>::value>::type>
    : private _T
{
public:
  using _T::operator();

protected:
  template <class... _Args> explicit __executor_wrapper_base_wrapped(_Args&&... __args)
    : _T(forward<_Args>(__args)...)
  {
  }

  _T& _Wrapped() { return *this; }
  const _T& _Wrapped() const { return *this; }
};

template <class _T>
class __executor_wrapper_base_wrapped<_T,
  typename enable_if<is_class<_T>::value && !__is_callable<_T>::value>::type>
    : private _T
{
protected:
  template <class... _Args> explicit __executor_wrapper_base_wrapped(_Args&&... __args)
    : _T(forward<_Args>(__args)...)
  {
  }

  _T& _Wrapped() { return *this; }
  const _T& _Wrapped() const { return *this; }
};

template <class _T>
class __executor_wrapper_base_wrapped<_T,
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

  template <class... _Args> explicit __executor_wrapper_base_wrapped(_Args&&... __args)
    : _M_wrapped(forward<_Args>(__args)...)
  {
  }

  _T& _Wrapped() { return _M_wrapped; }
  const _T& _Wrapped() const { return _M_wrapped; }

private:
  _T _M_wrapped;
};

template <class _T>
class __executor_wrapper_base_wrapped<_T,
  typename enable_if<!is_class<_T>::value && !__is_callable<_T>::value>::type>
{
protected:
  template <class _U, class _E> friend class executor_wrapper;

  template <class... _Args> explicit __executor_wrapper_base_wrapped(_Args&&... __args)
    : _M_wrapped(forward<_Args>(__args)...)
  {
  }

  _T& _Wrapped() { return _M_wrapped; }
  const _T& _Wrapped() const { return _M_wrapped; }

private:
  _T _M_wrapped;
};

} // namespace experimental
} // namespace std

#endif
