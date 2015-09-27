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

namespace std {
namespace experimental {
inline namespace concurrency_v2 {

template <class _Executor, class _T>
inline executor_wrapper<typename decay<_T>::type, _Executor>
  wrap(const _Executor& __e, _T&& __t,
    typename enable_if<is_executor<_Executor>::value>::type*)
{
  return executor_wrapper<typename decay<_T>::type, _Executor>(forward<_T>(__t), __e);
}

template <class _ExecutionContext, class _T>
inline executor_wrapper<typename decay<_T>::type, typename _ExecutionContext::executor_type>
wrap(_ExecutionContext& __c, _T&& __t,
  typename enable_if<is_convertible<
    _ExecutionContext&, execution_context&>::value>::type*)
{
  return executor_wrapper<typename decay<_T>::type,
    typename _ExecutionContext::executor_type>(forward<_T>(__t), __c.get_executor());
}

} // inline namespace concurrency_v2
} // namespace experimental
} // namespace std

#endif
