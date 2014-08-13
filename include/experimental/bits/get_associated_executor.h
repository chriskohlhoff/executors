//
// get_associated_executor.h
// ~~~~~~~~~~~~~~~~~~~~~~~~~
// Helper function to obtain an associated executor.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_GET_ASSOCIATED_EXECUTOR_H
#define EXECUTORS_EXPERIMENTAL_BITS_GET_ASSOCIATED_EXECUTOR_H

#include <experimental/bits/executor_traits.h>

namespace std {
namespace experimental {
inline namespace concurrency_v1 {

template <class _T, class _Executor>
struct __get_associated_executor_with_executor_result
{
  typedef associated_executor_t<_T, _Executor> _Result;
};

template <class _T, class _ExecutionContext>
struct __get_associated_executor_with_execution_context_result
{
  typedef associated_executor_t<_T, typename _ExecutionContext::executor_type> _Result;
};

struct __get_associated_executor_no_result {};

template <class _T, class _Executor>
struct __get_associated_executor_with_executor
  : conditional<__is_executor<_Executor>::value,
    __get_associated_executor_with_executor_result<_T, _Executor>, __get_associated_executor_no_result>::type
{
};

template <class _T, class _ExecutionContext>
struct __get_associated_executor_with_execution_context
  : conditional<__is_execution_context<_ExecutionContext>::value,
    __get_associated_executor_with_execution_context_result<_T, _ExecutionContext>, __get_associated_executor_no_result>::type
{
};

template <class _T>
inline associated_executor_t<_T> get_associated_executor(const _T& __t)
{
  return associated_executor<_T>::get(__t);
}

template <class _T, class _Executor>
inline typename __get_associated_executor_with_executor<_T, _Executor>::_Result
get_associated_executor(const _T& __t, const _Executor& __e)
{
  return associated_executor<_T, _Executor>::get(__t, __e);
}

template <class _T, class _ExecutionContext>
inline typename __get_associated_executor_with_execution_context<_T, _ExecutionContext>::_Result
get_associated_executor(const _T& __t, _ExecutionContext& __c)
{
  return associated_executor<_T, typename _ExecutionContext::executor_type>::get(__t, __c.get_executor());
}

} // inline namespace concurrency_v1
} // namespace experimental
} // namespace std

#endif
