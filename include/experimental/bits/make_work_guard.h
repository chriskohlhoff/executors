//
// make_work_guard.h
// ~~~~~~~~~~~~~~~~~
// Helper function to create work from an executor, execution context or object.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_MAKE_WORK_GUARD_H
#define EXECUTORS_EXPERIMENTAL_BITS_MAKE_WORK_GUARD_H

namespace std {
namespace experimental {
inline namespace concurrency_v2 {

template <class _Executor>
inline executor_work_guard<_Executor> make_work_guard(const _Executor& __e,
  typename enable_if<is_executor<_Executor>::value>::type*)
{
  return executor_work_guard<_Executor>(__e);
}

template <class _ExecutionContext>
inline executor_work_guard<typename _ExecutionContext::executor_type> make_work_guard(_ExecutionContext& __c,
  typename enable_if<is_convertible<_ExecutionContext&, execution_context&>::value>::type*)
{
  return executor_work_guard<typename _ExecutionContext::executor_type>(__c.get_executor());
}

template <class _T>
inline executor_work_guard<associated_executor_t<_T>> make_work_guard(const _T& __t,
  typename enable_if<!is_executor<_T>::value &&
    !is_convertible<_T&, execution_context&>::value>::type*)
{
  return executor_work_guard<associated_executor_t<_T>>(associated_executor<_T>::get(__t));
}

template <class _T, class _U>
inline auto make_work_guard(const _T& __t, _U&& __u)
  -> decltype(make_work_guard(get_associated_executor(__t, forward<_U>(__u))))
{
  return make_work_guard(get_associated_executor(__t, forward<_U>(__u)));
}

} // inline namespace concurrency_v2
} // namespace experimental
} // namespace std

#endif
