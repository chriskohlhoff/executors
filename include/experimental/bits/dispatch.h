//
// dispatch.h
// ~~~~~~~~~~
// Schedule a function to run now if possible, otherwise later.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_DISPATCH_H
#define EXECUTORS_EXPERIMENTAL_BITS_DISPATCH_H

#include <experimental/bits/invoker.h>
#include <experimental/bits/signature_type.h>

namespace std {
namespace experimental {

template <class _CompletionToken>
auto dispatch(_CompletionToken&& __token)
{
  return std::experimental::dispatch(__empty_function_void0(),
    forward<_CompletionToken>(__token));
}

template <class _Func, class _CompletionToken>
auto dispatch(_Func&& __f, _CompletionToken&& __token)
{
  typedef typename decay<_Func>::type _DecayFunc;
  typedef typename result_of<_DecayFunc()>::type _Result;
  typedef __signature_type_t<_Result> _Signature;
  typedef handler_type_t<_CompletionToken, _Signature> _Handler;

  async_completion<_CompletionToken, _Signature> __completion(__token);

  auto __completion_executor(get_executor(__completion.handler));
  auto __work(__completion_executor.make_work());

  __completion_executor.dispatch(
    __invoker<decltype(__work), typename decay<_Result>::type, _DecayFunc, _Handler>{
      __work, forward<_Func>(__f), std::move(__completion.handler)});

  return __completion.result.get();
}

template <class _Func, class _Executor, class _CompletionToken>
auto dispatch(_Func&& __f, _Executor&& __e, _CompletionToken&& __token)
{
  typedef typename decay<_Func>::type _DecayFunc;
  typedef typename result_of<_DecayFunc()>::type _Result;
  typedef __signature_type_t<_Result> _Signature;
  typedef handler_type_t<_CompletionToken, _Signature> _Handler;

  async_completion<_CompletionToken, _Signature> __completion(__token);

  auto __completion_executor(get_executor(__completion.handler));
  auto __work(__completion_executor.make_work());

  __e.dispatch(
    __invoker<decltype(__work), typename decay<_Result>::type, _DecayFunc, _Handler>{
      __work, forward<_Func>(__f), std::move(__completion.handler)});

  return __completion.result.get();
}

} // namespace experimental
} // namespace std

#endif
