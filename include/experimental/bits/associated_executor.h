//
// associated_executor.h
// ~~~~~~~~~~~~~~
// Obtain an object's associated executor.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_ASSOCIATED_EXECUTOR_H
#define EXECUTORS_EXPERIMENTAL_BITS_ASSOCIATED_EXECUTOR_H

#include <type_traits>

namespace std {
namespace experimental {
inline namespace concurrency_v1 {

template <class>
struct __associated_executor_check
{
  typedef void _Type;
};

template <class _T, class _E, class = void>
struct __associated_executor
{
  typedef _E _Type;

  static _Type _Get(const _T&, const _E& __e) noexcept
  {
    return __e;
  }
};

template <class _T, class _E>
struct __associated_executor<_T, _E,
  typename __associated_executor_check<typename _T::executor_type>::_Type>
{
  typedef typename _T::executor_type _Type;

  static _Type _Get(const _T& __t, const _E&) noexcept
  {
    return __t.get_executor();
  }
};

} // inline namespace concurrency_v1
} // namespace experimental
} // namespace std

#endif
