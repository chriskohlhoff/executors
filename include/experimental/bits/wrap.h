//
// wrap.h
// ~~~~~~
// Associate an executor with an object.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_WRAP_H
#define EXECUTORS_EXPERIMENTAL_BITS_WRAP_H

#include <experimental/bits/executor_traits.h>

namespace std {
namespace experimental {
inline namespace concurrency_v1 {

template <class _Executor, class _T>
struct __wrap_with_executor_result
{
  typedef executor_wrapper<typename decay<_T>::type, _Executor> _Result;
};

template <class _ExecutionContext, class _T>
struct __wrap_with_execution_context_result
{
  typedef executor_wrapper<typename decay<_T>::type,
    typename _ExecutionContext::executor_type> _Result;
};

struct __wrap_no_result {};

template <class _Executor, class _T>
struct __wrap_with_executor
  : conditional<__is_executor<_Executor>::value,
    __wrap_with_executor_result<_Executor, _T>, __wrap_no_result>::type
{
};

template <class _ExecutionContext, class _T>
struct __wrap_with_execution_context
  : conditional<__is_execution_context<_ExecutionContext>::value,
    __wrap_with_execution_context_result<_ExecutionContext, _T>, __wrap_no_result>::type
{
};

template <class _Executor, class _T>
inline typename __wrap_with_executor<_Executor, _T>::_Result
  wrap(const _Executor& __e, _T&& __t)
{
  return executor_wrapper<typename decay<_T>::type, _Executor>(forward<_T>(__t), __e);
}

template <class _ExecutionContext, class _T>
inline typename __wrap_with_execution_context<_ExecutionContext, _T>::_Result
  wrap(_ExecutionContext& __c, _T&& __t)
{
  return executor_wrapper<typename decay<_T>::type,
    typename _ExecutionContext::executor_type>(forward<_T>(__t), __c.get_executor());
}

} // inline namespace concurrency_v1
} // namespace experimental
} // namespace std

#endif
