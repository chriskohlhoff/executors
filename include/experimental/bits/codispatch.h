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

struct __coinvoke_dispatch
{
  template <class _E, class _F> void operator()(_E& __e, _F&& __f)
  {
    __e.dispatch(forward<_F>(__f));
  }
};

template <class... _CompletionTokens>
inline typename __coinvoke_without_executor<_CompletionTokens...>::_Result
  codispatch(_CompletionTokens&&... __tokens)
{
  return codispatch<sizeof...(_CompletionTokens) - 1>(forward<_CompletionTokens>(__tokens)...);
}

template <size_t _N, class... _CompletionTokens>
typename __coinvoke_n_without_executor<_N, _CompletionTokens...>::_Result
  codispatch(_CompletionTokens&&... __tokens)
{
  typedef __tuple_split_first<tuple<_CompletionTokens...>, _N> _Head;
  typedef __tuple_split_second<tuple<_CompletionTokens...>, _N> _Tail;
  return __coinvoker_launcher<_Head, _Tail>(__tokens...)._Go(__coinvoke_dispatch(), __tokens...);
}

template <class _Executor>
struct __coinvoke_dispatch_ex
{
  typename remove_reference<_Executor>::type& __e;

  template <class _E, class _F> void operator()(_E&, _F&& __f)
  {
    __e.dispatch(forward<_F>(__f));
  }
};

template <class _Executor, class... _CompletionTokens>
inline typename __coinvoke_with_executor<_Executor, _CompletionTokens...>::_Result
  codispatch(_Executor&& __e, _CompletionTokens&&... __tokens)
{
  return codispatch<sizeof...(_CompletionTokens) - 1>(forward<_Executor>(__e), forward<_CompletionTokens>(__tokens)...);
}

template <size_t _N, class _Executor, class... _CompletionTokens>
typename __coinvoke_n_with_executor<_N, _Executor, _CompletionTokens...>::_Result
  codispatch(_Executor&& __e, _CompletionTokens&&... __tokens)
{
  typedef __tuple_split_first<tuple<_CompletionTokens...>, _N> _Head;
  typedef __tuple_split_second<tuple<_CompletionTokens...>, _N> _Tail;
  return __coinvoker_launcher<_Head, _Tail>(__tokens...)._Go(__coinvoke_dispatch_ex<_Executor>{__e}, __tokens...);
}

} // namespace experimental
} // namespace std

#endif
