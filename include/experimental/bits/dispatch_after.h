//
// dispatch_after.h
// ~~~~~~~~~~~~~~~~
// Schedule a function to run at a relative time.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_DISPATCH_AFTER_H
#define EXECUTORS_EXPERIMENTAL_BITS_DISPATCH_AFTER_H

#include <experimental/bits/invoker.h>
#include <experimental/bits/timed_invoker.h>

namespace std {
namespace experimental {

template <class _Rep, class _Period, class... _CompletionTokens>
typename __invoke_without_executor<_CompletionTokens...>::_Result
  dispatch_after(const chrono::duration<_Rep, _Period>& __rel_time,
    _CompletionTokens&&... __tokens)
{
  static_assert(sizeof...(_CompletionTokens) > 0,
    "dispatch_after() must be called with one or more completion tokens");

  __timed_invoker<chrono::steady_clock, _CompletionTokens...> __head(__tokens...);
  async_result<__invoker_tail<void(), _CompletionTokens...>> __result(__head._Get_tail());

  auto __completion_executor(__head._Get_tail().get_executor());
  __head._Start(__completion_executor, __rel_time);

  return __result.get();
}

template <class _Rep, class _Period, class _Executor, class... _CompletionTokens>
typename __invoke_with_executor<_Executor, _CompletionTokens...>::_Result
  dispatch_after(const chrono::duration<_Rep, _Period>& __rel_time,
    _Executor&& __e, _CompletionTokens&&... __tokens)
{
  static_assert(sizeof...(_CompletionTokens) > 0,
    "dispatch_after() must be called with one or more completion tokens");

  __timed_invoker<chrono::steady_clock, _CompletionTokens...> __head(__tokens...);
  async_result<__invoker_tail<void(), _CompletionTokens...>> __result(__head._Get_tail());

  __head._Start(__e, __rel_time);

  return __result.get();
}

} // namespace experimental
} // namespace std

#endif
