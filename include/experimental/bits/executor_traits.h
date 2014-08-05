//
// executor_traits.h
// ~~~~~~~~~~~~~~~~~
// Helper traits to do with executors.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_EXECUTOR_TRAITS_H
#define EXECUTORS_EXPERIMENTAL_BITS_EXECUTOR_TRAITS_H

#include <type_traits>

namespace std {
namespace experimental {
inline namespace concurrency_v1 {

template <class... _T> struct __is_executor;

template <class _T, class... _U> struct __is_executor<_T, _U...>
  : is_executor<typename remove_reference<_T>::type> {};

template <class... _T> struct __is_execution_context;

template <class _T, class... _U> struct __is_execution_context<_T, _U...>
  : is_convertible<typename remove_reference<_T>::type&, execution_context&> {};

} // inline namespace concurrency_v1
} // namespace experimental
} // namespace std

#endif
