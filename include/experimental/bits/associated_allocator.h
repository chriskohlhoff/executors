//
// associated_allocator.h
// ~~~~~~~~~~~~~~~~~~~~~~
// Obtain an object's associated allocator.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_ASSOCIATED_ALLOCATOR_H
#define EXECUTORS_EXPERIMENTAL_BITS_ASSOCIATED_ALLOCATOR_H

#include <type_traits>

namespace std {
namespace experimental {
inline namespace concurrency_v1 {

template <class>
struct __associated_allocator_check
{
  typedef void _Type;
};

template <class _T, class _A, class = void>
struct __associated_allocator
{
  typedef _A _Type;

  static _Type _Get(const _T&, const _A& __a) noexcept
  {
    return __a;
  }
};

template <class _T, class _A>
struct __associated_allocator<_T, _A,
  typename __associated_allocator_check<typename _T::allocator_type>::_Type>
{
  typedef typename _T::allocator_type _Type;

  static _Type _Get(const _T& __t, const _A&) noexcept
  {
    return __t.get_allocator();
  }
};

} // inline namespace concurrency_v1
} // namespace experimental
} // namespace std

#endif
