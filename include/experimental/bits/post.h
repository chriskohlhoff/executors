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

template <class... _CompletionTokens>
typename __invoke_without_executor<_CompletionTokens...>::_Result
  post(_CompletionTokens&&... __tokens)
{
  static_assert(sizeof...(_CompletionTokens) > 0,
    "post() must be called with one or more completion tokens");

  __invoker_head<void(), _CompletionTokens...> __head(__tokens...);
  async_result<__invoker_head<void(), _CompletionTokens...>> __result(__head);

  auto __completion_executor(__head.get_executor());
  __completion_executor.post(std::move(__head), std::allocator<void>());

  return __result.get();
}

template <class _Executor, class... _CompletionTokens>
typename __invoke_with_executor<_Executor, _CompletionTokens...>::_Result
  post(_Executor&& __e, _CompletionTokens&&... __tokens)
{
  static_assert(sizeof...(_CompletionTokens) > 0,
    "post() must be called with one or more completion tokens");

  __invoker_head<void(), _CompletionTokens...> __head(__tokens...);
  async_result<__invoker_head<void(), _CompletionTokens...>> __result(__head);

  __e.post(std::move(__head), std::allocator<void>());

  return __result.get();
}

} // namespace experimental
} // namespace std

#endif
