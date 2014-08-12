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

#include <experimental/bits/executor_traits.h>

namespace std {
namespace experimental {
inline namespace concurrency_v1 {

template <class _Executor>
struct __work_with_executor_result
{
  typedef executor_work<_Executor> _Result;
};

template <class _ExecutionContext>
struct __work_with_execution_context_result
{
  typedef executor_work<typename _ExecutionContext::executor_type> _Result;
};

template <class _T>
struct __work_with_object_result
{
  typedef executor_work<associated_executor_t<_T>> _Result;
};

template <class _T, class _Executor>
struct __work_with_object_and_executor_result
{
  typedef executor_work<associated_executor_t<_T, _Executor>> _Result;
};

template <class _T, class _ExecutionContext>
struct __work_with_object_and_execution_context_result
{
  typedef executor_work<associated_executor_t<_T, typename _ExecutionContext::executor_type>> _Result;
};

struct __work_no_result {};

template <class _Executor>
struct __work_with_executor
  : conditional<__is_executor<_Executor>::value,
    __work_with_executor_result<_Executor>, __work_no_result>::type
{
};

template <class _ExecutionContext>
struct __work_with_execution_context
  : conditional<__is_execution_context<_ExecutionContext>::value,
    __work_with_execution_context_result<_ExecutionContext>, __work_no_result>::type
{
};

template <class _T>
struct __work_with_object
  : conditional<!__is_executor<_T>::value && !__is_execution_context<_T>::value,
    __work_with_object_result<_T>, __work_no_result>::type
{
};

template <class _T, class _Executor>
struct __work_with_object_and_executor
  : conditional<__is_executor<_Executor>::value,
    __work_with_object_and_executor_result<_T, _Executor>, __work_no_result>::type
{
};

template <class _T, class _ExecutionContext>
struct __work_with_object_and_execution_context
  : conditional<__is_execution_context<_ExecutionContext>::value,
    __work_with_object_and_execution_context_result<_T, _ExecutionContext>, __work_no_result>::type
{
};

template <class _Executor>
inline typename __work_with_executor<_Executor>::_Result
make_work(const _Executor& __e)
{
  return executor_work<_Executor>(__e);
}

template <class _ExecutionContext>
inline typename __work_with_execution_context<_ExecutionContext>::_Result
make_work(_ExecutionContext& __c)
{
  return executor_work<typename _ExecutionContext::executor_type>(__c.get_executor());
}

template <class _T>
inline typename __work_with_object<_T>::_Result
make_work(const _T& __t)
{
  return executor_work<associated_executor_t<_T>>(associated_executor<_T>::get(__t));
}

template <class _T, class _Executor>
inline typename __work_with_object_and_executor<_T, _Executor>::_Result
make_work(const _T& __t, const _Executor& __e)
{
  return executor_work<associated_executor_t<_T, _Executor>>(associated_executor<_T, _Executor>::get(__t, __e));
}

template <class _T, class _ExecutionContext>
inline typename __work_with_object_and_execution_context<_T, _ExecutionContext>::_Result
make_work(const _T& __t, _ExecutionContext& __c)
{
  return executor_work<associated_executor_t<_T, typename _ExecutionContext::executor_type>>(
    associated_executor<_T, typename _ExecutionContext::executor_type>::get(__t, __c.get_executor()));
}

} // inline namespace concurrency_v1
} // namespace experimental
} // namespace std

#endif
