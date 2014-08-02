//
// codefer.h
// ~~~~~~~~~~
// Schedule functions to run concurrently later.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_CODEFER_H
#define EXECUTORS_EXPERIMENTAL_BITS_CODEFER_H

#include <experimental/bits/coinvoker.h>

namespace std {
namespace experimental {
inline namespace concurrency_v1 {

struct __coinvoke_defer
{
  template <class _E, class _F, class _A>
  void operator()(_E& __e, _F&& __f, const _A& __a)
  {
    __e.defer(forward<_F>(__f), __a);
  }
};

template <class... _CompletionTokens>
inline typename __coinvoke_without_executor<_CompletionTokens...>::_Result
  codefer(_CompletionTokens&&... __tokens)
{
  constexpr size_t _N = sizeof...(_CompletionTokens) - 1;
  typedef __tuple_split_first<tuple<_CompletionTokens...>, _N> _Head;
  typedef __tuple_split_second<tuple<_CompletionTokens...>, _N> _Tail;
  return __coinvoker_launcher<_Head, _Tail>(__tokens...)._Go(__coinvoke_defer(), __tokens...);
}

template <class _Executor>
struct __coinvoke_defer_ex
{
  typename decay<_Executor>::type __e;

  template <class _E, class _F, class _A>
  void operator()(_E&, _F&& __f, const _A& __a)
  {
    __e.defer(forward<_F>(__f), __a);
  }
};

template <class _Executor, class... _CompletionTokens>
inline typename __coinvoke_with_executor<_Executor, _CompletionTokens...>::_Result
  codefer(const _Executor& __e, _CompletionTokens&&... __tokens)
{
  constexpr size_t _N = sizeof...(_CompletionTokens) - 1;
  typedef __tuple_split_first<tuple<_CompletionTokens...>, _N> _Head;
  typedef __tuple_split_second<tuple<_CompletionTokens...>, _N> _Tail;
  return __coinvoker_launcher<_Head, _Tail>(__tokens...)._Go(__coinvoke_defer_ex<_Executor>{__e}, __tokens...);
}

} // inline namespace concurrency_v1
} // namespace experimental
} // namespace std

#endif
