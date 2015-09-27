//
// is_executor.h
// ~~~~~~~~~~~~~~
// Trait to determine if a type meets the syntactic requirements of an executor.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_IS_EXECUTOR_H
#define EXECUTORS_EXPERIMENTAL_BITS_IS_EXECUTOR_H

#include <memory>
#include <type_traits>

namespace std {
namespace experimental {
inline namespace concurrency_v2 {

template <class>
struct __is_executor_check
{
  typedef void _Type;
};

template <class _T, class = void>
struct __is_executor : false_type {};

template <class _T>
struct __is_executor<_T,
  typename __is_executor_check<decltype(
    declval<_T>().context(),
    declval<_T>().on_work_started(),
    declval<_T>().on_work_finished())>::_Type> : true_type {};

template <class _T>
struct is_executor : __is_executor<_T> {};

} // inline namespace concurrency_v2
} // namespace experimental
} // namespace std

#endif
