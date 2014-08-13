//
// post.h
// ~~~~~~
// Schedule a function to run later.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_POST_H
#define EXECUTORS_EXPERIMENTAL_BITS_POST_H

#include <experimental/bits/invoker.h>

namespace std {
namespace experimental {
inline namespace concurrency_v1 {

template <class... _CompletionTokens>
typename __invoke_with_token<_CompletionTokens...>::_Result
  post(_CompletionTokens&&... __tokens)
{
  static_assert(sizeof...(_CompletionTokens) > 0,
    "post() must be called with one or more completion tokens");

  typedef __passive_invoker<void(), _CompletionTokens...> _Invoker;

  _Invoker __head(__tokens...);
  async_result<_Invoker> __result(__head);

  auto __completion_executor(get_associated_executor(__head));
  auto __completion_allocator(get_associated_allocator(__head));
  __completion_executor.post(std::move(__head), __completion_allocator);

  return __result.get();
}

template <class _Executor, class... _CompletionTokens>
typename __invoke_with_executor<_Executor, _CompletionTokens...>::_Result
  post(const _Executor& __e, _CompletionTokens&&... __tokens)
{
  static_assert(sizeof...(_CompletionTokens) > 0,
    "post() must be called with one or more completion tokens");

  typedef __active_invoker<void(), _CompletionTokens...> _Invoker;

  _Invoker __head(__tokens...);
  async_result<_Invoker> __result(__head);

  auto __completion_executor(__e);
  auto __completion_allocator(get_associated_allocator(__head));
  __completion_executor.post(std::move(__head), __completion_allocator);

  return __result.get();
}

template <class _ExecutionContext, class... _CompletionTokens>
typename __invoke_with_execution_context<_ExecutionContext, _CompletionTokens...>::_Result
  post(_ExecutionContext& __c, _CompletionTokens&&... __tokens)
{
  static_assert(sizeof...(_CompletionTokens) > 0,
    "post() must be called with one or more completion tokens");

  typedef __active_invoker<void(), _CompletionTokens...> _Invoker;

  _Invoker __head(__tokens...);
  async_result<_Invoker> __result(__head);

  auto __completion_executor(__c.get_executor());
  auto __completion_allocator(get_associated_allocator(__head));
  __completion_executor.post(std::move(__head), __completion_allocator);

  return __result.get();
}

} // inline namespace concurrency_v1
} // namespace experimental
} // namespace std

#endif
