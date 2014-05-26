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

#include <experimental/bits/function_traits.h>
#include <experimental/bits/get_executor.h>
#include <experimental/bits/tuple_utils.h>

namespace std {
namespace experimental {

template <class _Signature, class... _CompletionTokens> class __invoker_head;
template <class _Signature, class... _CompletionTokens> class __invoker_tail;

template <class _Result, class... _Args, class... _CompletionTokens>
class __invoker_tail<_Result(_Args...), _CompletionTokens...>
{
public:
  typedef __invoker_head<_Result(_Args...), _CompletionTokens...> _HeadInvoker;
  typedef typename _HeadInvoker::_Handler _Handler;
  typedef typename _HeadInvoker::_HandlerExecutor _HandlerExecutor;
  typedef typename _HeadInvoker::executor_type executor_type;

  explicit __invoker_tail(typename remove_reference<_CompletionTokens>::type&... __tokens)
    : _M_head(__tokens...), _M_work(_M_head._Get_handler_executor())
  {
  }

  __invoker_tail(_HeadInvoker&& __head, executor_work<_HandlerExecutor>&& __work)
    : _M_head(std::move(__head)), _M_work(std::move(__work))
  {
  }

  void operator()(_Args... __args) &&
  {
    auto ex(_M_work.get_executor());
    ex.dispatch(_Make_tuple_invoker(
      std::move(_M_head), forward<_Args>(__args)...),
        std::allocator<void>());
  }

  _Handler& _Get_handler()
  {
    return _M_head._Get_handler();
  }

  executor_type get_executor() const noexcept
  {
    return _M_head.get_executor();
  }

  template <class _C> auto _Chain(_C&& __c)
  {
    return __invoker_tail<_Result(_Args...), _CompletionTokens..., _C>(
      _M_head._Chain(forward<_C>(__c)), std::move(_M_work));
  }

  template <class _R, class... _A, class... _T>
  auto _Chain(__invoker_tail<_R(_A...), _T...>&& __c)
  {
    return __invoker_tail<_Result(_Args...), _CompletionTokens..., _T...>(
      _M_head._Chain(std::move(__c)), std::move(_M_work));
  }

private:
  _HeadInvoker _M_head;
  executor_work<_HandlerExecutor> _M_work;
};

template <class _Result, class... _Args, class _CompletionToken>
class __invoker_head<_Result(_Args...), _CompletionToken>
{
public:
  typedef handler_type_t<_CompletionToken, _Result(_Args...)> _Handler;
  typedef decltype(__get_executor_helper(declval<_Handler>())) _HandlerExecutor;
  typedef decltype(__get_executor_helper(declval<_Handler>())) executor_type;

  static_assert(__is_callable_with<_Handler, _Result(_Args...)>::value,
    "function object must be callable with the specified signature");

  explicit __invoker_head(typename remove_reference<_CompletionToken>::type& __token)
    : _M_handler(static_cast<_CompletionToken&&>(__token))
  {
  }

  void operator()(_Args... __args) &&
  {
    std::move(_M_handler)(forward<_Args>(__args)...);
  }

  _Handler& _Get_handler()
  {
    return _M_handler;
  }

  _HandlerExecutor _Get_handler_executor() const noexcept
  {
    return __get_executor_helper(_M_handler);
  }

  executor_type get_executor() const noexcept
  {
    return __get_executor_helper(_M_handler);
  }

  template <class _C> auto _Chain(_C&& __c)
  {
    return __invoker_head<_Result(_Args...), _CompletionToken, _C>(std::move(_M_handler),
      __invoker_tail<typename continuation_of<_Handler>::signature, _C>(__c));
  }

  template <class _R, class... _A, class... _T>
  auto _Chain(__invoker_tail<_R(_A...), _T...>&& __c)
  {
    return __invoker_head<_Result(_Args...), _CompletionToken, _T...>(
      std::move(_M_handler), std::move(__c));
  }

private:
  _Handler _M_handler;
};

template <class _Result, class... _Args, class _Head, class... _Tail>
class __invoker_head<_Result(_Args...), _Head, _Tail...>
{
public:
  typedef handler_type_t<_Head, _Result(_Args...)> _HeadFunc;
  typedef continuation_of<_HeadFunc> _HeadContinuation;
  typedef typename _HeadContinuation::signature _TailSignature;
  typedef __invoker_tail<_TailSignature, _Tail...> _TailInvoker;

  typedef typename _TailInvoker::_Handler _Handler;
  typedef decltype(__get_executor_helper(declval<_HeadFunc>())) _HandlerExecutor;
  typedef typename conditional<is_same<_HandlerExecutor, unspecified_executor>::value,
    typename _TailInvoker::executor_type, _HandlerExecutor>::type executor_type;

  __invoker_head(typename remove_reference<_Head>::type& __head,
    typename remove_reference<_Tail>::type&... __tail)
      : _M_head(static_cast<_Head&&>(__head)), _M_tail(__tail...)
  {
  }

  __invoker_head(_HeadFunc&& __head, _TailInvoker&& __tail)
    : _M_head(std::move(__head)), _M_tail(std::move(__tail))
  {
  }

  void operator()(_Args... __args) &&
  {
    _HeadContinuation::chain(std::move(_M_head), std::move(_M_tail))(forward<_Args>(__args)...);
  }

  _Handler& _Get_handler()
  {
    return _M_tail._Get_handler();
  }

  _HandlerExecutor _Get_handler_executor() const noexcept
  {
    return __get_executor_helper(_M_head);
  }

  executor_type get_executor() const noexcept
  {
    return get_executor(is_same<_HandlerExecutor, unspecified_executor>());
  }

  template <class _C> auto _Chain(_C&& __c)
  {
    return __invoker_head<_Result(_Args...), _Head, _Tail..., _C>(
      std::move(_M_head), _M_tail._Chain(forward<_C>(__c)));
  }

  template <class _R, class... _A, class... _T>
  auto _Chain(__invoker_tail<_R(_A...), _T...>&& __c)
  {
    return __invoker_head<_Result(_Args...), _Head, _Tail..., _T...>(
      std::move(_M_head), _M_tail._Chain(std::move(__c)));
  }

private:
  typename _TailInvoker::executor_type get_executor(true_type) const noexcept
  {
    return _M_tail.get_executor();
  }

  _HandlerExecutor get_executor(false_type) const noexcept
  {
    return __get_executor_helper(_M_head);
  }

  _HeadFunc _M_head;
  _TailInvoker _M_tail;
};

template <class _Result, class... _Args, class... _CompletionTokens>
struct continuation_of<__invoker_tail<_Result(_Args...), _CompletionTokens...>>
{
  typedef typename continuation_of<typename __invoker_tail<
    _Result(_Args...), _CompletionTokens...>::_Handler>::signature signature;

  template <class _C>
  static auto chain(__invoker_tail<_Result(_Args...), _CompletionTokens...>&& __f, _C&& __c)
  {
    return __f._Chain(forward<_C>(__c));
  }
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
