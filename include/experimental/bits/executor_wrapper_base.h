//
// executor_wrapper_base.h
// ~~~~~~~~~~~~~~~~~~~~~~~
// Wraps a function object so that it is associated with an executor.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_EXECUTOR_WRAPPER_BASE_H
#define EXECUTORS_EXPERIMENTAL_BITS_EXECUTOR_WRAPPER_BASE_H

#include <utility>

namespace std {
namespace experimental {
inline namespace concurrency_v1 {

template <class _Executor>
class __executor_wrapper_base_executor
{
protected:
  template <class... _Args> explicit __executor_wrapper_base_executor(_Args&&... __args)
    : _M_executor(forward<_Args>(__args)...)
  {
  }

  _Executor _M_executor;
};

template <class _T>
class __executor_wrapper_base_wrapped
{
protected:
  template <class... _Args> explicit __executor_wrapper_base_wrapped(_Args&&... __args)
    : _M_wrapped(forward<_Args>(__args)...)
  {
  }

  _T _M_wrapped;
};

} // inline namespace concurrency_v1
} // namespace experimental
} // namespace std

#endif
