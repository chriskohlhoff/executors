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
#include <memory>
#include <type_traits>
#include <experimental/bits/invoker.h>
#include <experimental/bits/tuple_utils.h>

namespace std {
namespace experimental {

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

template <class... _Head, class... _Tail>
class __coinvoker_tail<tuple<_Head...>, tuple<_Tail...>>
{
public:
  typedef __signature_cat_t<typename continuation_traits<
    handler_type_t<_Head, void()>>::signature...> _HandlerSignature;
  typedef __invoker_tail<_HandlerSignature, _Tail...> _TailInvoker;
  typedef typename _TailInvoker::_Handler _Handler;
  typedef typename _TailInvoker::_Executor _Executor;

  explicit __coinvoker_tail(typename remove_reference<_Tail>::type&... __token)
      : _M_pending(0), _M_invoker(__token...)
  {
  }

  void _Prime()
  {
    if (sizeof...(_Head) > 0)
    {
      _M_pending = sizeof...(_Head);
    }
    else
    {
      unique_ptr<__coinvoker_tail> __p(this);
      _Dispatch(typename _Make_index_sequence<sizeof...(_Head)>::_Type());
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
      _Dispatch(typename _Make_index_sequence<sizeof...(_Head)>::_Type());
    }
  }

  void _Release()
  {
    if (--_M_pending == 0)
      delete this;
  }

  _Handler& _Get_handler()
  {
    return _M_invoker._Get_handler();
  }

  _Executor _Make_initial_executor() const
  {
    return _M_invoker._Make_initial_executor();
  }

private:
  template <size_t... _Index>
  void _Dispatch(_Index_sequence<_Index...>)
  {
    _M_invoker(std::tuple_cat(get<_Index>(_M_results)._Get_value()...));
  }

  tuple<__coinvoker_result<typename continuation_traits<
    handler_type_t<_Head, void()>>::signature>...> _M_results;
  atomic<size_t> _M_pending;
  _TailInvoker _M_invoker;
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

  typename __coinvoker_tail<_Head, _Tail>::_Executor _Make_initial_executor() const
  {
    return _M_tail->_Make_initial_executor();
  }

private:
  __coinvoker_tail<_Head, _Tail>* _M_tail;
};

template <size_t _Index, class _Head, class _Tail>
class __coinvoker_head
{
public:
  typedef typename tuple_element<_Index, _Head>::type _HeadToken;
  typedef handler_type_t<_HeadToken, void()> _HeadFunc;
  typedef continuation_traits<_HeadFunc> _HeadTraits;

  typedef decltype(make_executor(declval<_HeadFunc>())) _HeadExecutor;
  typedef typename __coinvoker_tail<_Head, _Tail>::_Executor _TailExecutor;

  typedef typename conditional<
    is_same<_HeadExecutor, unspecified_executor>::value,
      _TailExecutor, _HeadExecutor>::type _Executor;

  __coinvoker_head(typename remove_reference<_HeadToken>::type& __token,
    __coinvoker_tail<_Head, _Tail>* __tail)
      : _M_head(static_cast<_HeadToken&&>(__token)), _M_tail(__tail)
  {
  }

  void operator()()
  {
    _HeadTraits::chain(std::move(_M_head), std::move(_M_tail))();
  }

  _Executor _Make_initial_executor() const
  {
    return _Make_initial_executor(is_same<_HeadExecutor, unspecified_executor>());
  }

private:
  _Executor _Make_initial_executor(true_type) const
  {
    return _M_tail._Make_initial_executor();
  }

  _Executor _Make_initial_executor(false_type) const
  {
    return make_executor(_M_head);
  }

  _HeadFunc _M_head;
  __coinvoker_tail_ptr<_Index, _Head, _Tail> _M_tail;
};

template <class _Head, class _Tail>
class __coinvoker_launcher;

template <class... _Head, class... _Tail>
class __coinvoker_launcher<tuple<_Head...>, tuple<_Tail...>>
{
public:
  __coinvoker_launcher(typename remove_reference<_Head>::type&...,
    typename remove_reference<_Tail>::type&... __tail)
      : _M_tail(new __coinvoker_tail<tuple<_Head...>, tuple<_Tail...>>(__tail...))
  {
  }

  template <class _Action>
  auto _Go(_Action __a, typename remove_reference<_Head>::type&... __head,
    typename remove_reference<_Tail>::type&...)
  {
    typedef typename __coinvoker_tail<tuple<_Head...>, tuple<_Tail...>>::_Handler _Handler;
    async_result<_Handler> __result(_M_tail->_Get_handler());
    this->_Go_0(__a, typename _Make_index_sequence<sizeof...(_Head)>::_Type(), __head...);
    return __result.get();
  }

private:
  template <class _Action, size_t... _Indexes>
  void _Go_0(_Action __a, _Index_sequence<_Indexes...>,
    typename remove_reference<_Head>::type&... __head)
  {
    this->_Go_1(__a,
      __coinvoker_head<_Indexes, tuple<_Head...>, tuple<_Tail...>>(
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
    auto __e(__i._Make_initial_executor());
    __a(__e, std::move(__i));
    (_Go_2)(__a, forward<_Invokers>(__j)...);
  }

  unique_ptr<__coinvoker_tail<tuple<_Head...>, tuple<_Tail...>>> _M_tail;
};

template <size_t _HeadSize, class... _CompletionTokens>
struct __coinvoke_result
{
  typedef __tuple_split_first<tuple<_CompletionTokens...>, _HeadSize> _Head;
  typedef __tuple_split_second<tuple<_CompletionTokens...>, _HeadSize> _Tail;
  typedef typename __coinvoker_tail<_Head, _Tail>::_Handler _Handler;
  typedef typename async_result<_Handler>::type _Result;
};

struct __coinvoke_no_result {};

template <size_t _HeadSize, class... _CompletionTokens>
struct __coinvoke_n_without_executor
  : conditional<__is_executor<_CompletionTokens...>::value,
    __coinvoke_no_result, __coinvoke_result<_HeadSize, _CompletionTokens...>>::type
{
};

template <size_t _HeadSize, class _Executor, class... _CompletionTokens>
struct __coinvoke_n_with_executor
  : conditional<__is_executor<_Executor>::value,
    __coinvoke_result<_HeadSize, _CompletionTokens...>, __coinvoke_no_result>::type
{
};

template <class... _CompletionTokens>
struct __coinvoke_without_executor
  : __coinvoke_n_without_executor<sizeof...(_CompletionTokens) - 1, _CompletionTokens...>
{
};

template <class _Executor, class... _CompletionTokens>
struct __coinvoke_with_executor
  : __coinvoke_n_with_executor<sizeof...(_CompletionTokens) - 1, _Executor, _CompletionTokens...>
{
};

} // namespace experimental
} // namespace std

#endif
