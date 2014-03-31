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
#include <experimental/bits/coinvoker.h>
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
class __coinvoker_handler;

template <class... _Head, class _Tail>
class __coinvoker_handler<tuple<_Head...>, tuple<_Tail>>
{
public:
  typedef __signature_cat_t<typename continuation_traits<
    handler_type_t<_Head, void()>>::signature...> _HandlerSignature;
  typedef handler_type_t<_Tail, _HandlerSignature> _Handler;

  explicit __coinvoker_handler(typename remove_reference<_Head>::type&...,
    typename remove_reference<_Tail>::type& __tail)
      : _M_pending(0), _M_handler(static_cast<_Tail&&>(__tail)),
        _M_handler_work(make_executor(_M_handler).make_work())
  {
  }

  template <size_t _Index, class... _Args> void _Set_result(_Args&&... __args)
  {
    get<_Index>(_M_results)._Set_value(forward<_Args>(__args)...);
  }

  void _Prime()
  {
    if (sizeof...(_Head) > 0)
    {
      _M_pending = sizeof...(_Head);
    }
    else
    {
      unique_ptr<__coinvoker_handler> __p(this);
      _Dispatch(typename _Make_index_sequence<sizeof...(_Head)>::_Type());
    }
  }

  void _Complete()
  {
    if (--_M_pending == 0)
    {
      unique_ptr<__coinvoker_handler> __p(this);
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
    return _M_handler;
  }

private:
  friend struct __coinvoker_handler_executor;

  template <size_t... _Index>
  void _Dispatch(_Index_sequence<_Index...>)
  {
    make_executor(_M_handler_work).dispatch(_Make_tuple_invoker(
      std::move(_M_handler), std::tuple_cat(get<_Index>(_M_results)._Get_value()...)));
  }

  tuple<__coinvoker_result<typename continuation_traits<
    handler_type_t<_Head, void()>>::signature>...> _M_results;
  atomic<int> _M_pending;
  _Handler _M_handler;
  typename decltype(make_executor(declval<_Handler>()))::work _M_handler_work;
};

template <size_t _Index, class _Head, class _Tail>
class __coinvoker
{
public:
  typedef typename __coinvoker_handler<_Head, _Tail>::_Handler _Handler;

  __coinvoker(const __coinvoker&) = delete;
  __coinvoker& operator=(const __coinvoker&) = delete;

  explicit __coinvoker(__coinvoker_handler<_Head, _Tail>* __h)
    : _M_handler(__h)
  {
  }

  __coinvoker(__coinvoker&& __f)
    : _M_handler(__f._M_handler)
  {
    __f._M_handler = 0;
  }

  ~__coinvoker()
  {
    if (_M_handler)
      _M_handler->_Release();
  }

  template <class... _Args> void operator()(_Args&&... __args)
  {
    _M_handler->template _Set_result<_Index>(forward<_Args>(__args)...);
    this->_Complete();
  }

  decltype(make_executor(declval<_Handler>())) _Get_executor() const
  {
    return make_executor(_M_handler->_M_handler);
  }

private:
  void _Complete()
  {
    auto* __h = _M_handler;
    _M_handler = 0;
    __h->_Complete();
  }

  __coinvoker_handler<_Head, _Tail>* _M_handler;
};

template <size_t _Index, class _Head, class _Tail>
inline auto make_executor(const __coinvoker<_Index, _Head, _Tail>& __i)
{
  return __i._Get_executor();
}

template <class _Head, class _Tail>
class __coinvoker_launcher;

template <class... _Head, class _Tail>
class __coinvoker_launcher<tuple<_Head...>, tuple<_Tail>>
{
public:
  __coinvoker_launcher(typename remove_reference<_Head>::type&... __head,
    typename remove_reference<_Tail>::type& __tail)
      : _M_handler(new __coinvoker_handler<tuple<_Head...>, tuple<_Tail>>(__head..., __tail))
  {
  }

  template <class _Action>
  auto _Go(_Action __a, typename remove_reference<_Head>::type&... __head,
    typename remove_reference<_Tail>::type&)
  {
    typedef typename __coinvoker_handler<tuple<_Head...>, tuple<_Tail>>::_Handler _Handler;
    async_result<_Handler> __result(_M_handler->_Get_handler());
    this->_Go_0(__a, typename _Make_index_sequence<sizeof...(_Head)>::_Type(), __head...);
    return __result.get();
  }

private:
  template <class _Action, size_t... _Indexes>
  void _Go_0(_Action __a, _Index_sequence<_Indexes...>,
    typename remove_reference<_Head>::type&... __head)
  {
    this->_Go_1(__a, continuation_traits<handler_type_t<_Head, void()>>::chain(
      handler_type_t<_Head, void()>(__head),
        __coinvoker<_Indexes, tuple<_Head...>, tuple<_Tail>>(_M_handler.get()))...);
  }

  template <class _Action, class... _Invokers>
  void _Go_1(_Action __a, _Invokers&&... __i)
  {
    _M_handler->_Prime();
    _M_handler.release();
    (_Go_2)(__a, forward<_Invokers>(__i)...);
  }

  template <class _Action> static void _Go_2(_Action&) {}

  template <class _Action, class _Invoker, class... _Invokers>
  static void _Go_2(_Action __a, _Invoker&& __i, _Invokers&&... __j)
  {
    __a(std::move(__i));
    (_Go_2)(__a, forward<_Invokers>(__j)...);
  }

  unique_ptr<__coinvoker_handler<tuple<_Head...>, tuple<_Tail>>> _M_handler;
};

template <class... _CompletionTokens>
struct __coinvoke_result
{
  static constexpr size_t _HeadSize = sizeof...(_CompletionTokens) - 1;
  typedef __tuple_split_first<tuple<_CompletionTokens...>, _HeadSize> _Head;
  typedef __tuple_split_second<tuple<_CompletionTokens...>, _HeadSize> _Tail;
  typedef typename __coinvoker_handler<_Head, _Tail>::_Handler _Handler;
  typedef typename async_result<_Handler>::type _Result;
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

} // namespace experimental
} // namespace std

#endif
