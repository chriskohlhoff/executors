//
// copost.h
// ~~~~~~~~
// Schedule functions to run concurrently later.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_COPOST_H
#define EXECUTORS_EXPERIMENTAL_BITS_COPOST_H

#include <experimental/bits/coinvoker.h>

namespace std {
namespace experimental {

struct __coinvoke_post
{
  template <class _E, class _F> void operator()(_E& __e, _F&& __f)
  {
    __e.post(forward<_F>(__f));
  }
};

template <class... _CompletionTokens>
typename __coinvoke_without_executor<_CompletionTokens...>::_Result
  copost(_CompletionTokens&&... __tokens)
{
  constexpr size_t _HeadSize = sizeof...(_CompletionTokens) - 1;
  typedef __tuple_split_first<tuple<_CompletionTokens...>, _HeadSize> _Head;
  typedef __tuple_split_second<tuple<_CompletionTokens...>, _HeadSize> _Tail;
  return __coinvoker_launcher<_Head, _Tail>(__tokens...)._Go(__coinvoke_post(), __tokens...);
}

template <class _Executor>
struct __coinvoke_post_ex
{
  typename remove_reference<_Executor>::type& __e;

  template <class _E, class _F> void operator()(_E&, _F&& __f)
  {
    __e.post(forward<_F>(__f));
  }
};

template <class _Executor, class... _CompletionTokens>
typename __coinvoke_with_executor<_Executor, _CompletionTokens...>::_Result
  copost(_Executor&& __e, _CompletionTokens&&... __tokens)
{
  constexpr size_t _HeadSize = sizeof...(_CompletionTokens) - 1;
  typedef __tuple_split_first<tuple<_CompletionTokens...>, _HeadSize> _Head;
  typedef __tuple_split_second<tuple<_CompletionTokens...>, _HeadSize> _Tail;
  return __coinvoker_launcher<_Head, _Tail>(__tokens...)._Go(__coinvoke_post_ex<_Executor>{__e}, __tokens...);
}

} // namespace experimental
} // namespace std

#endif
