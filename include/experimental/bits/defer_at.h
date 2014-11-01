//
// defer_at.h
// ~~~~~~~~~~
// Schedule a function to run at an absolute time.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_DEFER_AT_H
#define EXECUTORS_EXPERIMENTAL_BITS_DEFER_AT_H

#include <experimental/bits/timer_op.h>

namespace std {
namespace experimental {
inline namespace concurrency_v1 {

struct __defer_action
{
  template <class _Executor, class _Handler, class _Allocator>
  static void _Perform(_Executor& __e, _Handler&& __h, const _Allocator& __a)
  {
    __e.defer(forward<_Handler>(__h), __a);
  }
};

template <class _Clock, class _Duration, class _CompletionToken>
auto defer_at(const chrono::time_point<_Clock, _Duration>& __abs_time, _CompletionToken&& __token)
{
  typedef typename handler_type<_CompletionToken, void()>::type _Handler;
  typedef __timer_op<_Clock, associated_executor_t<_Handler>, __defer_action, _Handler> _Op;
  async_completion<_CompletionToken, void()> __completion(__token);
  auto __completion_executor((get_associated_executor)(__completion.handler));
  _Op::_Enqueue(__completion_executor, __abs_time, __completion.handler);
  return __completion.result.get();
}

template <class _Clock, class _Duration, class _Executor, class _CompletionToken>
inline auto defer_at(const chrono::time_point<_Clock, _Duration>& __abs_time,
  const _Executor& __e, _CompletionToken&& __token,
    typename enable_if<is_executor<_Executor>::value>::type*)
{
  typedef typename handler_type<_CompletionToken, void()>::type _Handler;
  typedef __timer_op<_Clock, _Executor, __defer_action, __work_dispatcher<_Handler>> _Op;
  async_completion<_CompletionToken, void()> __completion(__token);
  __work_dispatcher<_Handler> __d(__completion.handler);
  _Op::_Enqueue(__e, __abs_time, __d);
  return __completion.result.get();
}

template <class _Clock, class _Duration, class _ExecutionContext, class _CompletionToken>
auto defer_at(const chrono::time_point<_Clock, _Duration>& __abs_time,
  _ExecutionContext& __c, _CompletionToken&& __token,
    typename enable_if<is_convertible<
      _ExecutionContext&, execution_context&>::value>::type*)
{
  return (defer_at)(__abs_time, __c.get_executor(), forward<_CompletionToken>(__token));
}

} // inline namespace concurrency_v1
} // namespace experimental
} // namespace std

#endif
