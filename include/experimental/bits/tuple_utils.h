//
// tuple_utils.h
// ~~~~~~~~~~~~~
// Tuple helper utilities.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_TUPLE_UTILS_H
#define EXECUTORS_EXPERIMENTAL_BITS_TUPLE_UTILS_H

#include <tuple>

namespace std {
namespace experimental {
inline namespace concurrency_v1 {

// _Index_sequence: Used to expand tuples.

template <size_t... _I> struct _Index_sequence
{
  typedef _Index_sequence<_I..., sizeof...(_I)> _Next;
};

template <size_t _I> struct _Make_index_sequence
{
  typedef typename _Make_index_sequence<_I - 1>::_Type::_Next _Type;
};

template <> struct _Make_index_sequence<0>
{
  typedef _Index_sequence<> _Type;
};

// _Tuple_invoke: Invokes a function with arguments from a tuple.

template <class _Func, size_t... _I, class... _Values, class... _Args>
inline auto _Tuple_invoke_impl(_Func&& __f, _Index_sequence<_I...>,
  tuple<_Values...>& __values, _Args&&... __args)
{
  return forward<_Func>(__f)(get<_I>(__values)..., forward<_Args>(__args)...);
}

template <class _Func, size_t... _I, class... _Values, class... _Args>
inline auto _Tuple_invoke_impl(_Func&& __f, _Index_sequence<_I...>,
  tuple<_Values...>&& __values, _Args&&... __args)
{
  return forward<_Func>(__f)(std::move(get<_I>(__values))..., forward<_Args>(__args)...);
}

template <class _Func, class... _Values, class... _Args>
inline auto _Tuple_invoke(_Func&& __f, tuple<_Values...>& __values, _Args&&... __args)
{
  return _Tuple_invoke_impl(forward<_Func>(__f),
    typename _Make_index_sequence<sizeof...(_Values)>::_Type(),
      __values, forward<_Args>(__args)...);
}

template <class _Func, class... _Values, class... _Args>
inline auto _Tuple_invoke(_Func&& __f, tuple<_Values...>&& __values, _Args&&... __args)
{
  return _Tuple_invoke_impl(forward<_Func>(__f),
    typename _Make_index_sequence<sizeof...(_Values)>::_Type(),
      std::move(__values), forward<_Args>(__args)...);
}

template <class _Func, class... _Args>
inline auto _Tuple_invoke(_Func&& __f, tuple<>&, _Args&&... __args)
{
  return forward<_Func>(__f)(forward<_Args>(__args)...);
}

template <class _Func, class... _Args>
inline auto _Tuple_invoke(_Func&& __f, tuple<>&&, _Args&&... __args)
{
  return forward<_Func>(__f)(forward<_Args>(__args)...);
}

// _Make_tuple_invoker: Creates a function object to call a function using
// arguments from a tuple.

template <class _Handler, class... _Values>
struct __tuple_invoker
{
  _Handler _M_handler;
  tuple<_Values...> _M_args;

  void operator()()
  {
    _Tuple_invoke(std::move(_M_handler), std::move(_M_args));
  }
};

template <class _Handler, class... _Values>
inline auto _Make_tuple_invoker(_Handler&& __h, tuple<_Values...>&& __t)
{
  return __tuple_invoker<typename decay<_Handler>::type, _Values...>{
    forward<_Handler>(__h), std::move(__t)};
}

template <class _Handler, class... _Values>
inline auto _Make_tuple_invoker(_Handler&& __h, _Values&&... __v)
{
  return __tuple_invoker<typename decay<_Handler>::type, typename decay<_Values>::type...>{
    forward<_Handler>(__h), std::make_tuple(forward<_Values>(__v)...)};
}

// __args_tuple: Determines tuple type corresponding to a signature's argument list.

template <class _Signature>
struct __args_tuple;

template <class _R, class... _Args>
struct __args_tuple<_R(_Args...)>
{
  typedef tuple<_Args...> type;
};

template <class _Signature>
using __args_tuple_t = typename __args_tuple<_Signature>::type;

// _Tuple_get: Gets a return value out of a tuple.

template <class... _Values>
inline tuple<_Values...> _Tuple_get(tuple<_Values...>&& __t)
{
  return std::move(__t);
}

template <class _Value>
inline _Value _Tuple_get(tuple<_Value>&& __t)
{
  return get<0>(std::move(__t));
}

inline void _Tuple_get(tuple<>&&)
{
}

// __tuple_split: Splits a tuple into two types based on an index.

template <class _Tuple1, class _Tuple2, size_t _M, size_t _N, class = void>
struct __tuple_split_base;

template <class... _T, class _Head, class... _Tail, size_t _M, size_t _N>
struct __tuple_split_base<tuple<_T...>, tuple<_Head, _Tail...>, _M, _N, typename enable_if<(_M < _N)>::type>
  : __tuple_split_base<tuple<_T..., _Head>, tuple<_Tail...>, _M + 1, _N> {};

template <class... _T, class... _U, size_t _N>
struct __tuple_split_base<tuple<_T...>, tuple<_U...>, _N, _N>
{
  typedef tuple<_T...> first;
  typedef tuple<_U...> second;
};

template <class _Tuple, size_t _N>
struct __tuple_split : __tuple_split_base<tuple<>, _Tuple, 0, _N> {};

template <class _Tuple, size_t _N>
using __tuple_split_first = typename __tuple_split<_Tuple, _N>::first;

template <class _Tuple, size_t _N>
using __tuple_split_second = typename __tuple_split<_Tuple, _N>::second;

} // inline namespace concurrency_v1
} // namespace experimental
} // namespace std

#endif
