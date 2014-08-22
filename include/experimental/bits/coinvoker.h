//
// coinvoker.h
// ~~~~~~~~~~~
// Function objects used to implement copost, codispatch, etc.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_COINVOKER_H
#define EXECUTORS_EXPERIMENTAL_BITS_COINVOKER_H

#include <atomic>
#include <cassert>
#include <type_traits>
#include <experimental/memory>
#include <experimental/bits/invoker.h>
#include <experimental/bits/tuple_utils.h>

namespace std {
namespace experimental {
inline namespace concurrency_v1 {

template <class _Signature>
class __coinvoker_result
{
public:
  typedef __args_tuple_t<_Signature> _Result;

  __coinvoker_result(const __coinvoker_result&) = delete;
  __coinvoker_result& operator=(const __coinvoker_result&) = delete;

  __coinvoker_result()
    : _M_has_value(false)
  {
  }

  ~__coinvoker_result()
  {
    if (_M_has_value)
    {
      _Result* __v = static_cast<_Result*>(static_cast<void*>(&_M_value));
      __v->~_Result();
    }
  }

  template <class... _Args> void _Set_value(_Args&&... __args)
  {
    assert(!_M_has_value);
    new (&_M_value) _Result(forward<_Args>(__args)...);
    _M_has_value = true;
  }

  _Result _Get_value()
  {
    assert(_M_has_value);
    return std::move(*static_cast<_Result*>(static_cast<void*>(&_M_value)));
  }

private:
  typename aligned_storage<sizeof(_Result)>::type _M_value;
  bool _M_has_value;
};

template <class _Head, class _Tail>
class __coinvoker_tail;

template <class... _HeadTokens, class _TailToken>
class __coinvoker_tail<tuple<_HeadTokens...>, tuple<_TailToken>>
{
public:
  typedef __signature_cat_t<typename continuation_of<
    handler_type_t<_HeadTokens, void()>()>::signature...> _TailSignature;
  typedef __active_invoker<_TailSignature, _TailToken> _Invoker;

  explicit __coinvoker_tail(typename remove_reference<_TailToken>::type& __tail)
      : _M_pending(0), _M_invoker(__tail)
  {
  }

  void _Prime()
  {
    if (sizeof...(_HeadTokens) > 0)
    {
      _M_pending = sizeof...(_HeadTokens);
    }
    else
    {
      unique_ptr<__coinvoker_tail> __p(this);
      _Dispatch(typename _Make_index_sequence<sizeof...(_HeadTokens)>::_Type());
    }
  }

  template <size_t _Index, class... _Args> void _Set_result(_Args&&... __args)
  {
    get<_Index>(_M_results)._Set_value(forward<_Args>(__args)...);
  }

  void _Complete()
  {
    if (--_M_pending == 0)
    {
      unique_ptr<__coinvoker_tail> __p(this);
      _Dispatch(typename _Make_index_sequence<sizeof...(_HeadTokens)>::_Type());
    }
  }

  void _Release()
  {
    if (--_M_pending == 0)
      delete this;
  }

  _Invoker& _Get_invoker() noexcept
  {
    return _M_invoker;
  }

  const _Invoker& _Get_invoker() const noexcept
  {
    return _M_invoker;
  }

private:
  template <size_t... _Index>
  void _Dispatch(_Index_sequence<_Index...>)
  {
    _Tuple_invoke(std::move(_M_invoker), std::tuple_cat(get<_Index>(_M_results)._Get_value()...));
  }

  tuple<__coinvoker_result<typename continuation_of<
    handler_type_t<_HeadTokens, void()>()>::signature>...> _M_results;
  atomic<size_t> _M_pending;
  _Invoker _M_invoker;
};

template <size_t _Index, class _Head, class _Tail>
class __coinvoker_tail_ptr
{
public:
  __coinvoker_tail_ptr(const __coinvoker_tail_ptr&) = delete;
  __coinvoker_tail_ptr& operator=(const __coinvoker_tail_ptr&) = delete;

  explicit __coinvoker_tail_ptr(__coinvoker_tail<_Head, _Tail>* __t)
    : _M_tail(__t)
  {
  }

  __coinvoker_tail_ptr(__coinvoker_tail_ptr&& __p)
    : _M_tail(__p._M_tail)
  {
    __p._M_tail = 0;
  }

  ~__coinvoker_tail_ptr()
  {
    if (_M_tail)
      _M_tail->_Release();
  }

  template <class... _Args> void operator()(_Args&&... __args)
  {
    _M_tail->template _Set_result<_Index>(forward<_Args>(__args)...);
    auto* __t = _M_tail;
    _M_tail = 0;
    __t->_Complete();
  }

  typename __coinvoker_tail<_Head, _Tail>::_Invoker& _Get_invoker() noexcept
  {
    return _M_tail->_Get_invoker();
  }

  const typename __coinvoker_tail<_Head, _Tail>::_Invoker& _Get_invoker() const noexcept
  {
    return _M_tail->_Get_invoker();
  }

private:
  __coinvoker_tail<_Head, _Tail>* _M_tail;
};

template <size_t _Index, class _Head, class _Tail>
class __coinvoker_head
{
public:
  typedef typename tuple_element<_Index, _Head>::type _HeadToken;
  typedef handler_type_t<_HeadToken, void()> _Handler;
  typedef typename __coinvoker_tail<_Head, _Tail>::_Invoker _TailInvoker;
  typedef associated_allocator_t<_TailInvoker> _TailAllocator;
  typedef associated_allocator_t<_Handler, _TailAllocator> _Allocator;
  typedef associated_executor_t<_TailInvoker> _TailExecutor;
  typedef associated_executor_t<_Handler, _TailExecutor> _Executor;

  __coinvoker_head(typename remove_reference<_HeadToken>::type& __token,
    __coinvoker_tail<_Head, _Tail>* __tail)
      : _M_handler(static_cast<_HeadToken&&>(__token)), _M_tail(__tail)
  {
  }

  void operator()()
  {
    continuation_of<_Handler()>::chain(std::move(_M_handler), std::move(_M_tail))();
  }

  _Allocator _Get_allocator() const noexcept
  {
    return associated_allocator<_Handler, _TailAllocator>::get(
      _M_handler, associated_allocator<_TailInvoker>::get(_M_tail._Get_invoker()));
  }

  _Executor _Get_executor() const noexcept
  {
    return associated_executor<_Handler, _TailExecutor>::get(
      _M_handler, associated_executor<_TailInvoker>::get(_M_tail._Get_invoker()));
  }

private:
  _Handler _M_handler;
  __coinvoker_tail_ptr<_Index, _Head, _Tail> _M_tail;
};

template <class _Head, class _Tail>
class __coinvoker_launcher;

template <class... _HeadTokens, class _TailToken>
class __coinvoker_launcher<tuple<_HeadTokens...>, tuple<_TailToken>>
{
public:
  __coinvoker_launcher(typename remove_reference<_HeadTokens>::type&...,
    typename remove_reference<_TailToken>::type& __tail)
      : _M_tail(new __coinvoker_tail<tuple<_HeadTokens...>, tuple<_TailToken>>(__tail))
  {
  }

  template <class _Action>
  auto _Go(_Action __a, typename remove_reference<_HeadTokens>::type&... __head,
    typename remove_reference<_TailToken>::type&)
  {
    typedef typename __coinvoker_tail<tuple<_HeadTokens...>, tuple<_TailToken>>::_Invoker _Invoker;
    async_result<_Invoker> __result(_M_tail->_Get_invoker());
    this->_Go_0(__a, typename _Make_index_sequence<sizeof...(_HeadTokens)>::_Type(), __head...);
    return __result.get();
  }

private:
  template <class _Action, size_t... _Indexes>
  void _Go_0(_Action __a, _Index_sequence<_Indexes...>,
    typename remove_reference<_HeadTokens>::type&... __head)
  {
    this->_Go_1(__a,
      __coinvoker_head<_Indexes, tuple<_HeadTokens...>, tuple<_TailToken>>(
        __head, _M_tail.get())...);
  }

  template <class _Action, class... _Invokers>
  void _Go_1(_Action __a, _Invokers&&... __i)
  {
    _M_tail->_Prime();
    _M_tail.release();
    (_Go_2)(__a, forward<_Invokers>(__i)...);
  }

  template <class _Action> static void _Go_2(_Action&) {}

  template <class _Action, class _Invoker, class... _Invokers>
  static void _Go_2(_Action __a, _Invoker&& __i, _Invokers&&... __j)
  {
    auto __executor(__i._Get_executor());
    auto __allocator(__i._Get_allocator());
    __a(__executor, std::move(__i), __allocator);
    (_Go_2)(__a, forward<_Invokers>(__j)...);
  }

  unique_ptr<__coinvoker_tail<tuple<_HeadTokens...>, tuple<_TailToken>>> _M_tail;
};

template <class... _CompletionTokens>
struct __coinvoke_result
{
  static constexpr size_t _N = sizeof...(_CompletionTokens) - 1;
  typedef __tuple_split_first<tuple<_CompletionTokens...>, _N> _Head;
  typedef __tuple_split_second<tuple<_CompletionTokens...>, _N> _Tail;
  typedef typename __coinvoker_tail<_Head, _Tail>::_Invoker _Invoker;
  typedef typename async_result<_Invoker>::type _Result;
};

struct __coinvoke_no_result {};

template <class... _CompletionTokens>
struct __coinvoke_without_executor
  : conditional<__is_executor<_CompletionTokens...>::value,
    __coinvoke_no_result, __coinvoke_result<_CompletionTokens...>>::type
{
};

template <class _Executor, class... _CompletionTokens>
struct __coinvoke_with_executor
  : conditional<__is_executor<_Executor>::value,
    __coinvoke_result<_CompletionTokens...>, __coinvoke_no_result>::type
{
};

} // inline namespace concurrency_v1
} // namespace experimental
} // namespace std

#endif
