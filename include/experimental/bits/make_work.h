//
// make_work.h
// ~~~~~~~~~~~
// Helper function to create work from an executor, execution context or object.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_MAKE_WORK_H
#define EXECUTORS_EXPERIMENTAL_BITS_MAKE_WORK_H

namespace std {
namespace experimental {
inline namespace concurrency_v2 {

template <class _Executor>
inline executor_work<_Executor> make_work(const _Executor& __e,
  typename enable_if<is_executor<_Executor>::value>::type*)
{
  return executor_work<_Executor>(__e);
}

template <class _ExecutionContext>
inline executor_work<typename _ExecutionContext::executor_type> make_work(_ExecutionContext& __c,
  typename enable_if<is_convertible<_ExecutionContext&, execution_context&>::value>::type*)
{
  return executor_work<typename _ExecutionContext::executor_type>(__c.get_executor());
}

template <class _T>
inline executor_work<associated_executor_t<_T>> make_work(const _T& __t,
  typename enable_if<!is_executor<_T>::value &&
    !is_convertible<_T&, execution_context&>::value>::type*)
{
  return executor_work<associated_executor_t<_T>>(associated_executor<_T>::get(__t));
}

template <class _T, class _Executor>
inline executor_work<associated_executor_t<_T, _Executor>> make_work(const _T& __t, const _Executor& __e,
  typename enable_if<is_executor<_Executor>::value>::type*)
{
  return executor_work<associated_executor_t<_T, _Executor>>(associated_executor<_T, _Executor>::get(__t, __e));
}

template <class _T, class _ExecutionContext>
inline executor_work<associated_executor_t<_T, typename _ExecutionContext::executor_type>>
make_work(const _T& __t, _ExecutionContext& __c,
  typename enable_if<is_convertible<_ExecutionContext&, execution_context&>::value>::type*)
{
  return executor_work<associated_executor_t<_T, typename _ExecutionContext::executor_type>>(
    associated_executor<_T, typename _ExecutionContext::executor_type>::get(__t, __c.get_executor()));
}

} // inline namespace concurrency_v2
} // namespace experimental
} // namespace std

#endif
