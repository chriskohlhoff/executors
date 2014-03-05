//
// get_executor.h
// ~~~~~~~~~~~~~~
// Function to obtain associated executor.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_GET_EXECUTOR_H
#define EXECUTORS_EXPERIMENTAL_BITS_GET_EXECUTOR_H

namespace std {
namespace experimental {

template <class _Func>
inline system_executor get_executor(_Func&&)
{
  return system_executor();
}

} // namespace experimental
} // namespace std

#endif
