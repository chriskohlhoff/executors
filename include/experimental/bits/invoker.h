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
#include <experimental/bits/executor_traits.h>
#include <experimental/bits/function_traits.h>
#include <experimental/bits/tuple_utils.h>

namespace std {
namespace experimental {
inline namespace concurrency_v1 {

template <class _Signature, class... _CompletionTokens> class __passive_invoker;
template <class _Signature, class... _CompletionTokens> class __active_invoker;

template <class _Result, class... _Args, class... _CompletionTokens>
class __active_invoker<_Result(_Args...), _CompletionTokens...>
{
public:
  typedef __passive_invoker<_Result(_Args...), _CompletionTokens...> _Passive;
  typedef typename _Passive::_Handler _Handler;
  typedef typename _Passive::_TailSignature _TailSignature;

  explicit __active_invoker(typename remove_reference<_CompletionTokens>::type&... __tokens)
    : _M_passive(__tokens...), _M_work(associated_executor<_Passive>::get(_M_passive))
  {
  }

  __active_invoker(_Passive&& __passive, executor_work<associated_executor_t<_Passive>>&& __work)
    : _M_passive(std::move(__passive)), _M_work(std::move(__work))
  {
  }

  void operator()(_Args... __args)
  {
    auto __ex(_M_work.get_executor());
    auto __alloc(associated_allocator<_Passive>::get(_M_passive));
    __ex.dispatch(_Make_tuple_invoker(std::move(_M_passive), forward<_Args>(__args)...), __alloc);
  }

  _Passive& _Get_passive() noexcept
  {
    return _M_passive;
  }

  const _Passive& _Get_passive() const noexcept
  {
    return _M_passive;
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
  executor_work<associated_executor_t<_Passive>> _M_work;
};

template <class _Signature, class... _CompletionTokens>
class async_result<__active_invoker<_Signature, _CompletionTokens...>>
  : public async_result<typename __active_invoker<_Signature, _CompletionTokens...>::_Passive>
{
public:
  async_result(__active_invoker<_Signature, _CompletionTokens...>& __h)
    : async_result<typename __active_invoker<_Signature, _CompletionTokens...>::_Passive>(
        __h._Get_passive()) {}
};

template <class _Signature, class... _CompletionTokens, class _Alloc>
struct associated_allocator<__active_invoker<_Signature, _CompletionTokens...>, _Alloc>
{
  typedef __active_invoker<_Signature, _CompletionTokens...> _Invoker;
  typedef associated_allocator<typename _Invoker::_Passive, _Alloc> _PassiveTraits;
  typedef typename _PassiveTraits::type type;

  static type get(const _Invoker& __i, const _Alloc& __a = _Alloc()) noexcept
  {
    return _PassiveTraits::get(__i._Get_passive(), __a);
  }
};

template <class _Signature, class... _CompletionTokens, class _Executor>
struct associated_executor<__active_invoker<_Signature, _CompletionTokens...>, _Executor>
{
  typedef __active_invoker<_Signature, _CompletionTokens...> _Invoker;
  typedef associated_executor<typename _Invoker::_Passive, _Executor> _PassiveTraits;
  typedef typename _PassiveTraits::type type;

  static type get(const _Invoker& __i, const _Executor& __e = _Executor()) noexcept
  {
    return _PassiveTraits::get(__i._Get_passive(), __e);
  }
};

template <class _Result, class... _Args, class _CompletionToken>
class __passive_invoker<_Result(_Args...), _CompletionToken>
{
public:
  typedef handler_type_t<_CompletionToken, _Result(_Args...)> _Handler;
  typedef typename continuation_of<_Handler(_Args...)>::signature _TailSignature;

  static_assert(__is_callable_with<_Handler, _Result(_Args...)>::value,
    "function object must be callable with the specified signature");

  explicit __passive_invoker(typename remove_reference<_CompletionToken>::type& __token)
    : _M_handler(static_cast<_CompletionToken&&>(__token))
  {
  }

  void operator()(_Args... __args)
  {
    std::move(_M_handler)(forward<_Args>(__args)...);
  }

  _Handler& _Get_handler() noexcept
  {
    return _M_handler;
  }

  const _Handler& _Get_handler() const noexcept
  {
    return _M_handler;
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

template <class _Result, class... _Args, class _CompletionToken>
class async_result<__passive_invoker<_Result(_Args...), _CompletionToken>>
  : public async_result<typename __passive_invoker<_Result(_Args...), _CompletionToken>::_Handler>
{
public:
  async_result(__passive_invoker<_Result(_Args...), _CompletionToken>& __h)
    : async_result<typename __passive_invoker<_Result(_Args...), _CompletionToken>::_Handler>(
        __h._Get_handler()) {}
};

template <class _Result, class... _Args, class _CompletionToken, class _Alloc>
struct associated_allocator<__passive_invoker<_Result(_Args...), _CompletionToken>, _Alloc>
{
  typedef __passive_invoker<_Result(_Args...), _CompletionToken> _Invoker;
  typedef associated_allocator<typename _Invoker::_Handler, _Alloc> _HandlerTraits;
  typedef typename _HandlerTraits::type type;

  static type get(const _Invoker& __i, const _Alloc& __a = _Alloc()) noexcept
  {
    return _HandlerTraits::get(__i._Get_handler(), __a);
  }
};

template <class _Result, class... _Args, class _CompletionToken, class _Executor>
struct associated_executor<__passive_invoker<_Result(_Args...), _CompletionToken>, _Executor>
{
  typedef __passive_invoker<_Result(_Args...), _CompletionToken> _Invoker;
  typedef associated_executor<typename _Invoker::_Handler, _Executor> _HandlerTraits;
  typedef typename _HandlerTraits::type type;

  static type get(const _Invoker& __i, const _Executor& __e = _Executor()) noexcept
  {
    return _HandlerTraits::get(__i._Get_handler(), __e);
  }
};

template <class _Result, class... _Args, class _HeadToken, class... _TailTokens>
class __passive_invoker<_Result(_Args...), _HeadToken, _TailTokens...>
{
public:
  typedef handler_type_t<_HeadToken, _Result(_Args...)> _Handler;
  typedef typename continuation_of<_Handler(_Args...)>::signature _TailSignature;
  typedef __active_invoker<_TailSignature, _TailTokens...> _Tail;

  __passive_invoker(typename remove_reference<_HeadToken>::type& __head,
    typename remove_reference<_TailTokens>::type&... __tail)
      : _M_handler(static_cast<_HeadToken&&>(__head)), _M_tail(__tail...)
  {
  }

  __passive_invoker(_Handler&& __handler, _Tail&& __tail)
    : _M_handler(std::move(__handler)), _M_tail(std::move(__tail))
  {
  }

  void operator()(_Args... __args)
  {
    continuation_of<_Handler(_Args...)>::chain(std::move(_M_handler), std::move(_M_tail))(forward<_Args>(__args)...);
  }

  const _Handler& _Get_handler() const noexcept
  {
    return _M_handler;
  }

  _Tail& _Get_tail() noexcept
  {
    return _M_tail;
  }

  const _Tail& _Get_tail() const noexcept
  {
    return _M_tail;
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
  _Handler _M_handler;
  _Tail _M_tail;
};

template <class _Signature, class... _CompletionTokens>
class async_result<__passive_invoker<_Signature, _CompletionTokens...>>
  : public async_result<typename __passive_invoker<_Signature, _CompletionTokens...>::_Tail>
{
public:
  async_result(__passive_invoker<_Signature, _CompletionTokens...>& __h)
    : async_result<typename __passive_invoker<_Signature, _CompletionTokens...>::_Tail>(
        __h._Get_tail()) {}
};

template <class _Signature, class... _CompletionTokens, class _Alloc>
struct associated_allocator<__passive_invoker<_Signature, _CompletionTokens...>, _Alloc>
{
  typedef __passive_invoker<_Signature, _CompletionTokens...> _Invoker;
  typedef associated_allocator<typename _Invoker::_Tail, _Alloc> _TailTraits;
  typedef associated_allocator<typename _Invoker::_Handler, typename _TailTraits::type> _HandlerTraits;
  typedef typename _HandlerTraits::type type;

  static type get(const _Invoker& __i, const _Alloc& __a = _Alloc()) noexcept
  {
    return _HandlerTraits::get(__i._Get_handler(), _TailTraits::get(__i._Get_tail(), __a));
  }
};

template <class _Signature, class... _CompletionTokens, class _Executor>
struct associated_executor<__passive_invoker<_Signature, _CompletionTokens...>, _Executor>
{
  typedef __passive_invoker<_Signature, _CompletionTokens...> _Invoker;
  typedef associated_executor<typename _Invoker::_Tail, _Executor> _TailTraits;
  typedef associated_executor<typename _Invoker::_Handler, typename _TailTraits::type> _HandlerTraits;
  typedef typename _HandlerTraits::type type;

  static type get(const _Invoker& __i, const _Executor& __e = _Executor()) noexcept
  {
//    char* p = _TailTraits::get(__i._Get_tail(), __e);
    return _HandlerTraits::get(__i._Get_handler(), _TailTraits::get(__i._Get_tail(), __e));
  }
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

} // inline namespace concurrency_v1
} // namespace experimental
} // namespace std

#endif
