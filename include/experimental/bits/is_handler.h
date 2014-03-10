//
// is_handler.h
// ~~~~~~~~~~~~
// Type trait to determine whether a type is likely to be a handler type.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_IS_HANDLER_H
#define EXECUTORS_EXPERIMENTAL_BITS_IS_HANDLER_H

#include <type_traits>

namespace std {
namespace experimental {

struct __is_handler_base { void operator()(); };

template <class _T>
struct __is_handler_derived : _T, __is_handler_base {};

template <class _T, _T>
struct __is_handler_check
{
  typedef void _Type;
};

template <class _T, class = void>
struct __is_handler_class : true_type {};

template <class _T>
struct __is_handler_class<_T,
  typename __is_handler_check<void (__is_handler_base::*)(),
    &__is_handler_derived<_T>::operator()>::_Type> : false_type {};

template <class _T>
struct __is_handler_function :
  is_function<typename remove_pointer<typename remove_reference<_T>::type>::type> {};

template <class _T>
struct __is_handler :
  conditional<is_class<_T>::value,
    __is_handler_class<_T>, __is_handler_function<_T>>::type {};

} // namespace experimental
} // namespace std

#endif
