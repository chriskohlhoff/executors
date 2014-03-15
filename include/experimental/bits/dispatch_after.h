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

#include <experimental/bits/timed_invoker.h>
#include <experimental/bits/signature_type.h>

namespace std {
namespace experimental {

template <class _Rep, class _Period, class _CompletionToken>
auto dispatch_after(const chrono::duration<_Rep, _Period>& __rel_time,
  _CompletionToken&& __token)
{
  typedef handler_type_t<_CompletionToken, void()> _Handler;
  async_completion<_CompletionToken, void()> __completion(__token);

  auto __completion_executor(get_executor(__completion.handler));
  __timed_invoker<chrono::steady_clock, _Handler>(__completion_executor.context(),
    __rel_time, std::move(__completion.handler))._Start(__completion_executor);

  return __completion.result.get();
}

template <class _Rep, class _Period, class _Func, class _CompletionToken>
auto dispatch_after(const chrono::duration<_Rep, _Period>& __rel_time,
  _Func&& __f, _CompletionToken&& __token)
{
  typedef typename decay<_Func>::type _DecayFunc;
  typedef typename result_of<_DecayFunc()>::type _Result;
  typedef typename decay<_Result>::type _DecayResult;
  typedef __signature_type_t<_Result> _Signature;

  typedef handler_type_t<_CompletionToken, _Signature> _Handler;
  async_completion<_CompletionToken, _Signature> __completion(__token);

  auto __completion_executor(get_executor(__completion.handler));
  (dispatch_after)(__rel_time, __invoker<_DecayResult, _DecayFunc, _Handler>{
    forward<_Func>(__f), std::move(__completion.handler), __completion_executor.make_work()});

  return __completion.result.get();
}

template <class _Rep, class _Period, class _Executor, class _Func, class _CompletionToken>
auto dispatch_after(const chrono::duration<_Rep, _Period>& __rel_time,
  _Executor&& __e, _Func&& __f, _CompletionToken&& __token)
{
  return (dispatch_after)(__rel_time, __e.wrap(forward<_Func>(__f)), forward<_CompletionToken>(__token));
}

} // namespace experimental
} // namespace std

#endif
