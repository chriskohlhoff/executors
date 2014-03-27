//
// codispatch.h
// ~~~~~~~~~~~~
// Schedule functions to run now if possible, otherwise run concurrently later.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_CODISPATCH_H
#define EXECUTORS_EXPERIMENTAL_BITS_CODISPATCH_H

#include <experimental/bits/coinvoker.h>

namespace std {
namespace experimental {

template <class _Func1, class _Func2, class _CompletionToken>
auto codispatch(_Func1&& __f1, _Func2&& __f2, _CompletionToken&& __token)
{
  typedef typename decay<_Func1>::type _DecayFunc1;
  typedef __signature_t<_DecayFunc1> _Func1Signature;
  typedef continuation_traits<_DecayFunc1> _Traits1;
  typedef typename _Traits1::result_type _Result1;

  typedef typename decay<_Func2>::type _DecayFunc2;
  typedef __signature_t<_DecayFunc2> _Func2Signature;
  typedef continuation_traits<_DecayFunc2> _Traits2;
  typedef typename _Traits2::result_type _Result2;

  typedef __make_signature_t<void, _Result1, _Result2> _HandlerSignature;
  typedef handler_type_t<_CompletionToken, _HandlerSignature> _Handler;

  async_completion<_CompletionToken, _HandlerSignature> __completion(__token);

  unique_ptr<__coinvoker_handler<_Result1, _Result2, _Handler>> __h(
    new __coinvoker_handler<_Result1, _Result2, _Handler>(std::move(__completion.handler)));

  auto __i1(_Traits1::chain(forward<_Func1>(__f1), __coinvoker<1, _Result1, _Result2, _Handler>(__h.get())));
  auto __i2(_Traits2::chain(forward<_Func2>(__f2), __coinvoker<2, _Result1, _Result2, _Handler>(__h.get())));

  __h->_Prime();
  __h.release();

  (dispatch)(std::move(__i1));
  (dispatch)(std::move(__i2));

  return __completion.result.get();
}

template <class _Executor, class _Func1, class _Func2, class _CompletionToken>
auto codispatch(_Executor&& __e, _Func1&& __f1, _Func2&& __f2, _CompletionToken&& __token)
{
  return (codispatch)(__e.wrap(forward<_Func1>(__f1)),
    __e.wrap(forward<_Func2>(__f2)), forward<_CompletionToken>(__token));
}

} // namespace experimental
} // namespace std

#endif
