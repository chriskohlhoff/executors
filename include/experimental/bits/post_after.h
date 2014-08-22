//
// post_after.h
// ~~~~~~~~~~~~
// Schedule a function to run at a relative time.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_POST_AFTER_H
#define EXECUTORS_EXPERIMENTAL_BITS_POST_AFTER_H

#include <experimental/bits/invoker.h>
#include <experimental/bits/timed_invoker.h>

namespace std {
namespace experimental {
inline namespace concurrency_v1 {

template <class _Rep, class _Period, class... _CompletionTokens>
typename __invoke_with_token<_CompletionTokens...>::_Result
  post_after(const chrono::duration<_Rep, _Period>& __rel_time,
    _CompletionTokens&&... __tokens)
{
  static_assert(sizeof...(_CompletionTokens) > 0,
    "post_after() must be called with one or more completion tokens");

  typedef __timed_invoker<chrono::steady_clock, _CompletionTokens...> _Invoker;

  _Invoker __head(__tokens...);
  async_result<typename _Invoker::_Tail> __result(__head._Get_tail());

  auto __completion_executor(get_associated_executor(__head._Get_tail()));
  __head._Start(__completion_executor, __rel_time);

  return __result.get();
}

template <class _Rep, class _Period, class _Executor, class... _CompletionTokens>
typename __invoke_with_executor<_Executor, _CompletionTokens...>::_Result
  post_after(const chrono::duration<_Rep, _Period>& __rel_time,
    const _Executor& __e, _CompletionTokens&&... __tokens)
{
  static_assert(sizeof...(_CompletionTokens) > 0,
    "post_after() must be called with one or more completion tokens");

  typedef __timed_invoker<chrono::steady_clock, _CompletionTokens...> _Invoker;

  _Invoker __head(__tokens...);
  async_result<typename _Invoker::_Tail> __result(__head._Get_tail());

  __head._Start(__e, __rel_time);

  return __result.get();
}

template <class _Rep, class _Period, class _ExecutionContext, class... _CompletionTokens>
typename __invoke_with_execution_context<_ExecutionContext, _CompletionTokens...>::_Result
  post_after(const chrono::duration<_Rep, _Period>& __rel_time,
    _ExecutionContext& __c, _CompletionTokens&&... __tokens)
{
  static_assert(sizeof...(_CompletionTokens) > 0,
    "post_after() must be called with one or more completion tokens");

  typedef __timed_invoker<chrono::steady_clock, _CompletionTokens...> _Invoker;

  _Invoker __head(__tokens...);
  async_result<typename _Invoker::_Tail> __result(__head._Get_tail());

  __head._Start(__c.get_executor(), __rel_time);

  return __result.get();
}

} // inline namespace concurrency_v1
} // namespace experimental
} // namespace std

#endif
