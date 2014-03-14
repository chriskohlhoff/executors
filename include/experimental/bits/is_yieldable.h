//
// is_yieldable.h
// ~~~~~~~~~~~~~~
// Type trait to determine whether a type is a yieldable function.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_IS_YIELDABLE_H
#define EXECUTORS_EXPERIMENTAL_BITS_IS_YIELDABLE_H

#include <type_traits>

namespace std {
namespace experimental {

template <class _Executor> class basic_yield_context;

struct __is_yieldable_helper
{
  template <class _Executor> operator basic_yield_context<_Executor>();
};

template <class> struct __is_yieldable_check { typedef void type; };

template <class _T, class = void>
struct __is_yieldable_class_callable : false_type {};

template <class _T, class... _Args>
struct __is_yieldable_class_callable<_T(_Args...),
  typename result_of<_T(_Args..., __is_yieldable_helper)>::type> : true_type {};

template <class _T, class = void>
struct __is_yieldable_class : false_type {};

template <class _T, class... _Args>
struct __is_yieldable_class<_T(_Args...),
  typename __is_yieldable_check<decltype(&_T::operator())>::type>
    : __is_yieldable_class_callable<_T(_Args...)> {};

template <class _T, class = void>
struct __is_yieldable_function
  : public false_type {};

template <class _T, class... _Args>
struct __is_yieldable_function<_T(_Args...),
  typename result_of<typename decay<_T>::type(_Args..., __is_yieldable_helper)>::type>
    : public true_type {};

template <class _T>
struct __is_yieldable;

template <class _T, class... _Args>
struct __is_yieldable<_T(_Args...)>
  : conditional<is_class<_T>::value,
      __is_yieldable_class<_T(_Args...)>,
      __is_yieldable_function<_T(_Args...)>>::type {};

template <class _Arg, class... _Args>
struct __yield_last_argument_type : __yield_last_argument_type<_Args...> {};

template <class _Arg>
struct __yield_last_argument_type<_Arg> { typedef _Arg type; };

template <class _T>
struct __yield_argument_type_function;

template <class _R, class... _Args>
struct __yield_argument_type_function<_R(_Args...)>
  : __yield_last_argument_type<_Args...> {};

template <class _R, class... _Args>
struct __yield_argument_type_function<_R(*)(_Args...)>
  : __yield_last_argument_type<_Args...> {};

template <class _R, class... _Args>
struct __yield_argument_type_function<_R(&)(_Args...)>
  : __yield_last_argument_type<_Args...> {};

template <class _R, class _C, class... _Args>
struct __yield_argument_type_function<_R(_C::*)(_Args...)>
  : __yield_last_argument_type<_Args...> {};

template <class _R, class _C, class... _Args>
struct __yield_argument_type_function<_R(_C::*)(_Args...) const>
  : __yield_last_argument_type<_Args...> {};

template <class _T>
struct __yield_argument_type_class
  : __yield_argument_type_function<decltype(&_T::operator())> {};

template <class _T>
struct __yield_argument_type
  : conditional<is_class<_T>::value,
      __yield_argument_type_class<_T>,
      __yield_argument_type_function<_T>>::type {};

} // namespace experimental
} // namespace std

#endif
