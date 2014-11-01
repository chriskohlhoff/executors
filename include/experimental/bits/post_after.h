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

#include <experimental/bits/post_at.h>

namespace std {
namespace experimental {
inline namespace concurrency_v1 {

template <class _Rep, class _Period, class _CompletionToken>
inline auto post_after(const chrono::duration<_Rep, _Period>& __rel_time, _CompletionToken&& __token)
{
  return (post_at)(chrono::steady_clock::now() + __rel_time, forward<_CompletionToken>(__token));
}

template <class _Rep, class _Period, class _Executor, class _CompletionToken>
inline auto post_after(const chrono::duration<_Rep, _Period>& __rel_time,
  const _Executor& __e, _CompletionToken&& __token,
    typename enable_if<is_executor<_Executor>::value>::type*)
{
  return (post_at)(chrono::steady_clock::now() + __rel_time, __e, forward<_CompletionToken>(__token));
}

template <class _Rep, class _Period, class _ExecutionContext, class _CompletionToken>
inline auto post_after(const chrono::duration<_Rep, _Period>& __rel_time,
  _ExecutionContext& __c, _CompletionToken&& __token,
    typename enable_if<is_convertible<
      _ExecutionContext&, execution_context&>::value>::type*)
{
  return (post_at)(chrono::steady_clock::now() + __rel_time, __c, forward<_CompletionToken>(__token));
}

} // inline namespace concurrency_v1
} // namespace experimental
} // namespace std

#endif
