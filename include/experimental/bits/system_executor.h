//
// system_executor.h
// ~~~~~~~~~~~~~~~~~
// System executor implementation.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_SYSTEM_EXECUTOR_H
#define EXECUTORS_EXPERIMENTAL_BITS_SYSTEM_EXECUTOR_H

#include <cassert>
#include <type_traits>

namespace std {
namespace experimental {

template <class _Func>
inline void system_executor::post(_Func&& __f)
{
  assert(0 && "system_executor::post() not yet implemented");
  (void)__f;
}

template <class _Func>
void system_executor::dispatch(_Func&& __f)
{
  typename decay<_Func>::type tmp(forward<_Func>(__f));
  tmp();
}

inline system_executor::work system_executor::make_work()
{
  return work{};
}

} // namespace experimental
} // namespace std

#endif
