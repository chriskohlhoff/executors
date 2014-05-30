//
// unspecified_allocator.h
// ~~~~~~~~~~~~~~~~~~~~~~~
// Unspecified allocator implementation.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_UNSPECIFIED_ALLOCATOR_H
#define EXECUTORS_EXPERIMENTAL_BITS_UNSPECIFIED_ALLOCATOR_H

namespace std {
namespace experimental {

template <class _T>
inline unspecified_allocator<_T>::unspecified_allocator() noexcept
{
}

template <class _T>
inline unspecified_allocator<_T>::unspecified_allocator(
  const unspecified_allocator& __a) noexcept
    : std::allocator<_T>(static_cast<const std::allocator<_T>&>(__a))
{
}

template <class _T> template <class _U>
inline unspecified_allocator<_T>::unspecified_allocator(
  const unspecified_allocator<_U>& __a) noexcept
    : std::allocator<_T>(static_cast<const std::allocator<_U>&>(__a))
{
}

} // namespace experimental
} // namespace std

#endif
