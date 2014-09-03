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

#include <utility>

namespace std {
namespace experimental {
inline namespace concurrency_v1 {

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

template <class _T>
class __executor_wrapper_base_wrapped
{
protected:
  template <class... _Args> explicit __executor_wrapper_base_wrapped(_Args&&... __args)
    : _M_wrapped(forward<_Args>(__args)...)
  {
  }

  _T _M_wrapped;
};

template <class>
struct __executor_wrapper_base_check
{
  typedef void _Type;
};

template <class _T, class = void>
struct __executor_wrapper_base_argument_type
{
};

template <class _R, class _Arg>
struct __executor_wrapper_base_argument_type<_R(_Arg)>
{
  typedef _Arg argument_type;
};

template <class _R, class _Arg>
struct __executor_wrapper_base_argument_type<_R(*)(_Arg)>
{
  typedef _Arg argument_type;
};

template <class _R, class _Arg>
struct __executor_wrapper_base_argument_type<_R(&)(_Arg)>
{
  typedef _Arg argument_type;
};

template <class _T>
struct __executor_wrapper_base_argument_type<_T,
  typename __executor_wrapper_base_check<typename _T::argument_type>::_Type>
{
  typedef typename _T::argument_type argument_type;
};

template <class _T, class = void>
struct __executor_wrapper_base_argument_types
{
};

template <class _R, class _Arg1, class _Arg2>
struct __executor_wrapper_base_argument_types<_R(_Arg1, _Arg2)>
{
  typedef _Arg1 first_argument_type;
  typedef _Arg2 second_argument_type;
};

template <class _R, class _Arg1, class _Arg2>
struct __executor_wrapper_base_argument_types<_R(*)(_Arg1, _Arg2)>
{
  typedef _Arg1 first_argument_type;
  typedef _Arg2 second_argument_type;
};

template <class _R, class _Arg1, class _Arg2>
struct __executor_wrapper_base_argument_types<_R(&)(_Arg1, _Arg2)>
{
  typedef _Arg1 first_argument_type;
  typedef _Arg2 second_argument_type;
};

template <class _T>
struct __executor_wrapper_base_argument_types<_T,
  typename __executor_wrapper_base_check<typename _T::first_argument_type>::_Type>
{
  typedef typename _T::first_argument_type first_argument_type;
  typedef typename _T::second_argument_type second_argument_type;
};

} // inline namespace concurrency_v1
} // namespace experimental
} // namespace std

#endif
