//
// signature_type.h
// ~~~~~~~~~~~~~~~~
// Determine function signature type from an argument type.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_SIGNATURE_TYPE_H
#define EXECUTORS_EXPERIMENTAL_BITS_SIGNATURE_TYPE_H

namespace std {
namespace experimental {

template <class _Signature>
struct __signature_type_base { typedef _Signature _Type; };

template <class _T>
struct __signature_type : __signature_type_base<void(_T)> {};

template <>
struct __signature_type<void> : __signature_type_base<void()> {};

template <class _T>
using __signature_type_t = typename __signature_type<_T>::_Type;

} // namespace experimental
} // namespace std

#endif
