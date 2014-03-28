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

template <class _Func, size_t... _I, class... _Values, class... _Args>
inline auto _Tuple_invoke_impl(_Func&& __f, _Index_sequence<_I...>,
  tuple<_Values...>& __values, _Args&&... __args)
{
  return __f(get<_I>(__values)..., forward<_Args>(__args)...);
}

template <class _Func, class... _Values, class... _Args>
inline auto _Tuple_invoke(_Func&& __f, tuple<_Values...>& __values, _Args&&... __args)
{
  return _Tuple_invoke_impl(forward<_Func>(__f),
    typename _Make_index_sequence<sizeof...(_Args)>::_Type(),
      __values, forward<_Args>(__args)...);
}

template <class _Func, class... _Args>
inline auto _Tuple_invoke(_Func&& __f, tuple<>&, _Args&&... __args)
{
  __f(forward<_Args>(__args)...);
}

} // namespace experimental
} // namespace std

#endif
