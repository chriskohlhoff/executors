//
// get_allocator.h
// ~~~~~~~~~~~~~~~
// Obtain an object's associated allocator.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_GET_ALLOCATOR_H
#define EXECUTORS_EXPERIMENTAL_BITS_GET_ALLOCATOR_H

#include <type_traits>
#include <experimental/bits/function_traits.h>

namespace std {
namespace experimental {
inline namespace concurrency_v1 {

template <class>
struct __get_allocator_check
{
  typedef void _Type;
};

template <class _T, class = void>
struct __get_unspecified_allocator_impl {};

template <class _T>
struct __get_unspecified_allocator_impl<_T,
  typename enable_if<__is_callable<_T>::value>::type>
{
  typedef unspecified_allocator<void> _Type;

  static _Type _Get(const _T&) noexcept
  {
    return _Type();
  }
};

template <class _T, class>
struct __get_allocator_impl
  : __get_unspecified_allocator_impl<_T> {};

template <class _T>
struct __get_allocator_impl<_T,
  typename __get_allocator_check<typename _T::allocator_type>::_Type>
{
  typedef typename _T::allocator_type _Type;

  static _Type _Get(const _T& __t) noexcept
  {
    return __t.get_allocator();
  }
};

template <class _T>
inline typename __get_allocator_impl<_T>::_Type get_allocator(const _T& __t) noexcept
{
  return __get_allocator_impl<_T>::_Get(__t);
}

template <class _T>
inline auto __get_allocator_helper(const _T& __t)
{
  return get_allocator(__t);
}

template <class _T>
struct __is_unspecified_allocator
  : false_type {};

template <class _T>
struct __is_unspecified_allocator<unspecified_allocator<_T>>
  : true_type {};

} // inline namespace concurrency_v1
} // namespace experimental
} // namespace std

#endif
