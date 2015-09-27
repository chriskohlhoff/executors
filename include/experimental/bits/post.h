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

#include <experimental/bits/work_dispatcher.h>

namespace std {
namespace experimental {
inline namespace concurrency_v2 {

template <class _CompletionToken>
auto post(_CompletionToken&& __token)
{
  async_completion<_CompletionToken, void()> __completion(__token);
  auto __completion_executor(get_associated_executor(__completion.completion_handler));
  auto __completion_allocator(get_associated_allocator(__completion.completion_handler));
  __completion_executor.post(std::move(__completion.completion_handler), __completion_allocator);
  return __completion.result.get();
}

template <class _Executor, class _CompletionToken>
auto post(const _Executor& __e, _CompletionToken&& __token,
  typename enable_if<is_executor<_Executor>::value>::type*)
{
  typedef typename async_completion<_CompletionToken, void()>::completion_handler_type _Handler;
  async_completion<_CompletionToken, void()> __completion(__token);
  _Executor __completion_executor(__e);
  auto __completion_allocator(get_associated_allocator(__completion.completion_handler));
  __completion_executor.post(__work_dispatcher<_Handler>(__completion.completion_handler), __completion_allocator);
  return __completion.result.get();
}

template <class _ExecutionContext, class _CompletionToken>
inline auto post(_ExecutionContext& __c, _CompletionToken&& __token,
  typename enable_if<is_convertible<
    _ExecutionContext&, execution_context&>::value>::type*)
{
  return (post)(__c.get_executor(), forward<_CompletionToken>(__token));
}

} // inline namespace concurrency_v2
} // namespace experimental
} // namespace std

#endif
