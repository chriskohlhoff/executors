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

#include <experimental/bits/invoker.h>
#include <experimental/bits/timed_invoker.h>

namespace std {
namespace experimental {
inline namespace concurrency_v1 {

template <class _Clock, class _Duration, class... _CompletionTokens>
typename __invoke_with_token<_CompletionTokens...>::_Result
  defer_at(const chrono::time_point<_Clock, _Duration>& __abs_time,
    _CompletionTokens&&... __tokens)
{
  static_assert(sizeof...(_CompletionTokens) > 0,
    "defer_at() must be called with one or more completion tokens");

  __timed_invoker<_Clock, _CompletionTokens...> __head(__tokens...);
  async_result<__active_invoker<void(), _CompletionTokens...>> __result(__head._Get_tail());

  auto __completion_executor(__head._Get_tail().get_executor());
  __head._Start(__completion_executor, __abs_time);

  return __result.get();
}

template <class _Clock, class _Duration, class _Executor, class... _CompletionTokens>
typename __invoke_with_executor<_Executor, _CompletionTokens...>::_Result
  defer_at(const chrono::time_point<_Clock, _Duration>& __abs_time,
    const _Executor& __e, _CompletionTokens&&... __tokens)
{
  static_assert(sizeof...(_CompletionTokens) > 0,
    "defer_at() must be called with one or more completion tokens");

  __timed_invoker<_Clock, _CompletionTokens...> __head(__tokens...);
  async_result<__active_invoker<void(), _CompletionTokens...>> __result(__head._Get_tail());

  __head._Start(__e, __abs_time);

  return __result.get();
}

} // inline namespace concurrency_v1
} // namespace experimental
} // namespace std

#endif
