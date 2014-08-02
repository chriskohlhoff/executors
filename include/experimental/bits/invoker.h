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

#include <experimental/memory>
#include <experimental/bits/function_traits.h>
#include <experimental/bits/get_executor.h>
#include <experimental/bits/tuple_utils.h>

namespace std {
namespace experimental {

template <class _Signature, class... _CompletionTokens> class __passive_invoker;
template <class _Signature, class... _CompletionTokens> class __active_invoker;

template <class _Result, class... _Args, class... _CompletionTokens>
class __active_invoker<_Result(_Args...), _CompletionTokens...>
{
public:
  typedef __passive_invoker<_Result(_Args...), _CompletionTokens...> _Passive;
  typedef typename _Passive::_Handler _Handler;
  typedef typename _Passive::_TailSignature _TailSignature;
  typedef typename _Passive::_TerminalHandler _TerminalHandler;
  typedef typename _Passive::_HandlerExecutor _HandlerExecutor;
  typedef typename _Passive::executor_type executor_type;
  typedef typename _Passive::allocator_type allocator_type;

  explicit __active_invoker(typename remove_reference<_CompletionTokens>::type&... __tokens)
    : _M_passive(__tokens...), _M_work(_M_passive._Get_handler_executor())
  {
  }

  __active_invoker(_Passive&& __passive, executor_work<_HandlerExecutor>&& __work)
    : _M_passive(std::move(__passive)), _M_work(std::move(__work))
  {
  }

  void operator()(_Args... __args) &&
  {
    _HandlerExecutor __ex(_M_work.get_executor());
    allocator_type __alloc(_M_passive.get_allocator());
    __ex.dispatch(_Make_tuple_invoker(std::move(_M_passive), forward<_Args>(__args)...), __alloc);
  }

  _TerminalHandler& _Get_terminal_handler()
  {
    return _M_passive._Get_terminal_handler();
  }

  executor_type get_executor() const noexcept
  {
    return _M_passive.get_executor();
  }

  allocator_type get_allocator() const noexcept
  {
    return _M_passive.get_allocator();
  }

  template <class _C> auto _Chain(_C&& __c)
  {
    return __active_invoker<_Result(_Args...), _CompletionTokens..., _C>(
      _M_passive._Chain(forward<_C>(__c)), std::move(_M_work));
  }

  template <class _R, class... _A, class... _T>
  auto _Chain(__active_invoker<_R(_A...), _T...>&& __c)
  {
    return __active_invoker<_Result(_Args...), _CompletionTokens..., _T...>(
      _M_passive._Chain(std::move(__c)), std::move(_M_work));
  }

private:
  _Passive _M_passive;
  executor_work<_HandlerExecutor> _M_work;
};

template <class _Result, class... _Args, class _CompletionToken>
class __passive_invoker<_Result(_Args...), _CompletionToken>
{
public:
  typedef handler_type_t<_CompletionToken, _Result(_Args...)> _Handler;
  typedef typename continuation_of<_Handler(_Args...)>::signature _TailSignature;
  typedef _Handler _TerminalHandler;
  typedef decltype(__get_executor_helper(declval<_Handler>())) _HandlerExecutor;
  typedef _HandlerExecutor executor_type;
  typedef decltype(__get_allocator_helper(declval<_Handler>())) _HandlerAllocator;
  typedef _HandlerAllocator allocator_type;

  static_assert(__is_callable_with<_Handler, _Result(_Args...)>::value,
    "function object must be callable with the specified signature");

  explicit __passive_invoker(typename remove_reference<_CompletionToken>::type& __token)
    : _M_handler(static_cast<_CompletionToken&&>(__token))
  {
  }

  void operator()(_Args... __args) &&
  {
    std::move(_M_handler)(forward<_Args>(__args)...);
  }

  _TerminalHandler& _Get_terminal_handler()
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

  allocator_type get_allocator() const noexcept
  {
    return __get_allocator_helper(_M_handler);
  }

  template <class _C> auto _Chain(_C&& __c)
  {
    return __passive_invoker<_Result(_Args...), _CompletionToken, _C>(
      std::move(_M_handler), __active_invoker<_TailSignature, _C>(__c));
  }

  template <class _R, class... _A, class... _T>
  auto _Chain(__active_invoker<_R(_A...), _T...>&& __c)
  {
    return __passive_invoker<_Result(_Args...), _CompletionToken, _T...>(
      std::move(_M_handler), std::move(__c));
  }

private:
  _Handler _M_handler;
};

template <class _Result, class... _Args, class _HeadToken, class... _TailTokens>
class __passive_invoker<_Result(_Args...), _HeadToken, _TailTokens...>
{
public:
  typedef handler_type_t<_HeadToken, _Result(_Args...)> _Handler;
  typedef typename continuation_of<_Handler(_Args...)>::signature _TailSignature;
  typedef __active_invoker<_TailSignature, _TailTokens...> _Tail;
  typedef typename _Tail::_TerminalHandler _TerminalHandler;
  typedef decltype(__get_executor_helper(declval<_Handler>())) _HandlerExecutor;
  typedef typename conditional<is_same<_HandlerExecutor, unspecified_executor>::value,
    typename _Tail::executor_type, _HandlerExecutor>::type executor_type;
  typedef decltype(__get_allocator_helper(declval<_Handler>())) _HandlerAllocator;
  typedef typename conditional<__is_unspecified_allocator<_HandlerAllocator>::value,
    typename _Tail::allocator_type, _HandlerAllocator>::type allocator_type;

  __passive_invoker(typename remove_reference<_HeadToken>::type& __head,
    typename remove_reference<_TailTokens>::type&... __tail)
      : _M_handler(static_cast<_HeadToken&&>(__head)), _M_tail(__tail...)
  {
  }

  __passive_invoker(_Handler&& __handler, _Tail&& __tail)
    : _M_handler(std::move(__handler)), _M_tail(std::move(__tail))
  {
  }

  void operator()(_Args... __args) &&
  {
    continuation_of<_Handler(_Args...)>::chain(std::move(_M_handler), std::move(_M_tail))(forward<_Args>(__args)...);
  }

  _TerminalHandler& _Get_terminal_handler()
  {
    return _M_tail._Get_terminal_handler();
  }

  _HandlerExecutor _Get_handler_executor() const noexcept
  {
    return __get_executor_helper(_M_handler);
  }

  executor_type get_executor() const noexcept
  {
    return get_executor(is_same<_HandlerExecutor, unspecified_executor>());
  }

  allocator_type get_allocator() const noexcept
  {
    return get_allocator(__is_unspecified_allocator<_HandlerAllocator>());
  }

  template <class _C> auto _Chain(_C&& __c)
  {
    return __passive_invoker<_Result(_Args...), _HeadToken, _TailTokens..., _C>(
      std::move(_M_handler), _M_tail._Chain(forward<_C>(__c)));
  }

  template <class _R, class... _A, class... _T>
  auto _Chain(__active_invoker<_R(_A...), _T...>&& __c)
  {
    return __passive_invoker<_Result(_Args...), _HeadToken, _TailTokens..., _T...>(
      std::move(_M_handler), _M_tail._Chain(std::move(__c)));
  }

private:
  typename _Tail::executor_type get_executor(true_type) const noexcept
  {
    return _M_tail.get_executor();
  }

  _HandlerExecutor get_executor(false_type) const noexcept
  {
    return __get_executor_helper(_M_handler);
  }

  typename _Tail::allocator_type get_allocator(true_type) const noexcept
  {
    return _M_tail.get_allocator();
  }

  _HandlerAllocator get_allocator(false_type) const noexcept
  {
    return __get_allocator_helper(_M_handler);
  }

  _Handler _M_handler;
  _Tail _M_tail;
};

template <class _Result, class... _Args, class... _CompletionTokens, class... _Args2>
struct continuation_of<__active_invoker<_Result(_Args...), _CompletionTokens...>(_Args2...)>
{
  typedef typename __active_invoker<_Result(_Args...), _CompletionTokens...>::_TailSignature signature;

  template <class _C>
  static auto chain(__active_invoker<_Result(_Args...), _CompletionTokens...>&& __f, _C&& __c)
  {
    return __f._Chain(forward<_C>(__c));
  }
};

template <class _Signature, class... _CompletionTokens>
class async_result<__passive_invoker<_Signature, _CompletionTokens...>>
  : public async_result<typename __passive_invoker<_Signature, _CompletionTokens...>::_TerminalHandler>
{
public:
  async_result(__passive_invoker<_Signature, _CompletionTokens...>& __h)
    : async_result<typename __passive_invoker<_Signature, _CompletionTokens...>::_TerminalHandler>(
        __h._Get_terminal_handler()) {}
};

template <class _Signature, class... _CompletionTokens>
class async_result<__active_invoker<_Signature, _CompletionTokens...>>
  : public async_result<typename __active_invoker<_Signature, _CompletionTokens...>::_TerminalHandler>
{
public:
  async_result(__active_invoker<_Signature, _CompletionTokens...>& __h)
    : async_result<typename __active_invoker<_Signature, _CompletionTokens...>::_TerminalHandler>(
        __h._Get_terminal_handler()) {}
};

template <class... _T> struct __is_executor;

template <class _T, class... _U> struct __is_executor<_T, _U...>
  : is_executor<typename remove_reference<_T>::type> {};

template <class... _T> struct __is_execution_context;

template <class _T, class... _U> struct __is_execution_context<_T, _U...>
  : is_convertible<typename remove_reference<_T>::type&, execution_context&> {};

template <class... _CompletionTokens>
struct __invoke_result
{
  typedef typename async_result<__passive_invoker<void(), _CompletionTokens...>>::type _Result;
};

struct __invoke_no_result {};

template <class... _CompletionTokens>
struct __invoke_with_token
  : conditional<!__is_executor<_CompletionTokens...>::value
      && !__is_execution_context<_CompletionTokens...>::value,
    __invoke_result<_CompletionTokens...>, __invoke_no_result>::type
{
};

template <class _Executor, class... _CompletionTokens>
struct __invoke_with_executor
  : conditional<__is_executor<_Executor>::value,
    __invoke_result<_CompletionTokens...>, __invoke_no_result>::type
{
};

template <class _ExecutionContext, class... _CompletionTokens>
struct __invoke_with_execution_context
  : conditional<__is_execution_context<_ExecutionContext>::value,
    __invoke_result<_CompletionTokens...>, __invoke_no_result>::type
{
};

} // namespace experimental
} // namespace std

#endif
