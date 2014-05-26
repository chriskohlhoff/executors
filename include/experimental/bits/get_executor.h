//
// get_executor.h
// ~~~~~~~~~~~~~~
// Obtain an object's associated executor.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_GET_EXECUTOR_H
#define EXECUTORS_EXPERIMENTAL_BITS_GET_EXECUTOR_H

#include <type_traits>
#include <experimental/bits/function_traits.h>

namespace std {
namespace experimental {

template <class>
struct __get_executor_check
{
  typedef void _Type;
};

template <class _T, class = void>
struct __get_unspecified_executor_impl {};

template <class _T>
struct __get_unspecified_executor_impl<_T,
  typename enable_if<__is_callable<_T>::value>::type>
{
  typedef unspecified_executor _Type;

  static _Type _Get(const _T&) noexcept
  {
    return _Type();
  }
};

template <class _T, class>
struct __get_executor_impl
  : __get_unspecified_executor_impl<_T> {};

template <class _T>
struct __get_executor_impl<_T,
  typename __get_executor_check<typename _T::executor_type>::_Type>
{
  typedef typename _T::executor_type _Type;

  static _Type _Get(const _T& __t) noexcept
  {
    return __t.get_executor();
  }
};

template <class _T>
inline typename __get_executor_impl<_T>::_Type get_executor(const _T& __t) noexcept
{
  return __get_executor_impl<_T>::_Get(__t);
}

template <class _T>
inline auto __get_executor_helper(const _T& __t)
{
  return get_executor(__t);
}

} // namespace experimental
} // namespace std

#endif
