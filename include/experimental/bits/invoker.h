//
// invoker.h
// ~~~~~~~~~
// Function objects used to implement post, dispatch, etc.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_INVOKER_H
#define EXECUTORS_EXPERIMENTAL_BITS_INVOKER_H

#include <experimental/bits/tuple_utils.h>

namespace std {
namespace experimental {

template <class _Signature, class... _CompletionTokens>
class __invoker_head;

template <class _Signature, class... _CompletionTokens>
class __invoker_tail
{
public:
  typedef __invoker_head<_Signature, _CompletionTokens...> _HeadInvoker;
  typedef typename _HeadInvoker::_Handler _Handler;
  typedef typename _HeadInvoker::_Executor _Executor;
  typedef typename _HeadInvoker::_InitialExecutor _InitialExecutor;

  explicit __invoker_tail(typename remove_reference<_CompletionTokens>::type&... __tokens)
    : _M_head(__tokens...),
      _M_work(_M_head._Make_executor().make_work())
  {
  }

  template <class... _Args> void operator()(_Args&&... __args)
  {
    make_executor(_M_work).dispatch(_Make_tuple_invoker(
      std::move(_M_head), forward<_Args>(__args)...));
  }

  _Handler& _Get_handler()
  {
    return _M_head._Get_handler();
  }

  _InitialExecutor _Make_initial_executor() const
  {
    return _M_head._Make_initial_executor();
  }

private:
  _HeadInvoker _M_head;
  typename _Executor::work _M_work;
};

template <class _Signature, class _CompletionToken>
class __invoker_head<_Signature, _CompletionToken>
{
public:
  typedef handler_type_t<_CompletionToken, _Signature> _Handler;
  typedef decltype(make_executor(declval<_Handler>())) _Executor;
  typedef decltype(make_executor(declval<_Handler>())) _InitialExecutor;

  explicit __invoker_head(typename remove_reference<_CompletionToken>::type& __token)
    : _M_handler(static_cast<_CompletionToken&&>(__token))
  {
  }

  template <class... _Args> void operator()(_Args&&... __args)
  {
    _M_handler(forward<_Args>(__args)...);
  }

  _Handler& _Get_handler()
  {
    return _M_handler;
  }

  _Executor _Make_executor() const
  {
    return make_executor(_M_handler);
  }

  _InitialExecutor _Make_initial_executor() const
  {
    return make_executor(_M_handler);
  }

private:
  _Handler _M_handler;
};

template <class _Signature, class _Head, class... _Tail>
class __invoker_head<_Signature, _Head, _Tail...>
{
public:
  typedef handler_type_t<_Head, _Signature> _HeadFunc;
  typedef continuation_traits<_HeadFunc> _HeadTraits;
  typedef typename _HeadTraits::signature _TailSignature;
  typedef __invoker_tail<_TailSignature, _Tail...> _TailInvoker;

  typedef typename _TailInvoker::_Handler _Handler;
  typedef decltype(make_executor(declval<_HeadFunc>())) _Executor;
  typedef typename conditional<is_same<_Executor, unspecified_executor>::value,
    typename _TailInvoker::_InitialExecutor, _Executor>::type _InitialExecutor;

  __invoker_head(typename remove_reference<_Head>::type& __head,
    typename remove_reference<_Tail>::type&... __tail)
      : _M_head(static_cast<_Head&&>(__head)), _M_tail(__tail...)
  {
  }

  template <class... _Args> void operator()(_Args&&... __args)
  {
    _HeadTraits::chain(std::move(_M_head), std::move(_M_tail))(forward<_Args>(__args)...);
  }

  _Handler& _Get_handler()
  {
    return _M_tail._Get_handler();
  }

  _Executor _Make_executor() const
  {
    return make_executor(_M_head);
  }

  _InitialExecutor _Make_initial_executor() const
  {
    return _Make_initial_executor(is_same<_Executor, unspecified_executor>());
  }

private:
  _InitialExecutor _Make_initial_executor(true_type) const
  {
    return _M_tail._Make_initial_executor();
  }

  _InitialExecutor _Make_initial_executor(false_type) const
  {
    return make_executor(_M_head);
  }

  _HeadFunc _M_head;
  _TailInvoker _M_tail;
};

template <class _Signature, class... _CompletionTokens>
class async_result<__invoker_head<_Signature, _CompletionTokens...>>
  : public async_result<typename __invoker_head<_Signature, _CompletionTokens...>::_Handler>
{
public:
  async_result(__invoker_head<_Signature, _CompletionTokens...>& __h)
    : async_result<typename __invoker_head<_Signature, _CompletionTokens...>::_Handler>(
        __h._Get_handler()) {}
};

template <class _Signature, class... _CompletionTokens>
class async_result<__invoker_tail<_Signature, _CompletionTokens...>>
  : public async_result<typename __invoker_tail<_Signature, _CompletionTokens...>::_Handler>
{
public:
  async_result(__invoker_tail<_Signature, _CompletionTokens...>& __h)
    : async_result<typename __invoker_tail<_Signature, _CompletionTokens...>::_Handler>(
        __h._Get_handler()) {}
};

template <class... _T> struct __is_executor;

template <class _T, class... _U> struct __is_executor<_T, _U...>
  : is_executor<typename remove_reference<_T>::type> {};

template <class... _CompletionTokens>
struct __invoke_result
{
  typedef typename async_result<__invoker_head<void(), _CompletionTokens...>>::type _Result;
};

struct __invoke_no_result {};

template <class... _CompletionTokens>
struct __invoke_without_executor
  : conditional<__is_executor<_CompletionTokens...>::value,
    __invoke_no_result, __invoke_result<_CompletionTokens...>>::type
{
};

template <class _Executor, class... _CompletionTokens>
struct __invoke_with_executor
  : conditional<__is_executor<_Executor>::value,
    __invoke_result<_CompletionTokens...>, __invoke_no_result>::type
{
};

} // namespace experimental
} // namespace std

#endif
