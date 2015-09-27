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

namespace std {
namespace experimental {
inline namespace concurrency_v2 {

template <class _T>
inline associated_executor_t<_T> get_associated_executor(const _T& __t)
{
  return associated_executor<_T>::get(__t);
}

template <class _T, class _Executor>
inline associated_executor_t<_T, _Executor>
  get_associated_executor(const _T& __t, const _Executor& __e,
    typename enable_if<is_executor<_Executor>::value>::type*)
{
  return associated_executor<_T, _Executor>::get(__t, __e);
}

template <class _T, class _ExecutionContext>
inline associated_executor_t<_T, typename _ExecutionContext::executor_type>
  get_associated_executor(const _T& __t, _ExecutionContext& __c,
    typename enable_if<is_convertible<_ExecutionContext&, execution_context&>::value>::type*)
{
  return associated_executor<_T, typename _ExecutionContext::executor_type>::get(__t, __c.get_executor());
}

} // inline namespace concurrency_v2
} // namespace experimental
} // namespace std

#endif
