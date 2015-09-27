//
// executor_binder_base.h
// ~~~~~~~~~~~~~~~~~~~~~~
// Binds an executor to an object.
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
inline namespace concurrency_v2 {

template <class _Executor>
class __executor_binder_base_executor
{
protected:
  template <class... _Args> explicit __executor_binder_base_executor(_Args&&... __args)
    : _M_executor(forward<_Args>(__args)...)
  {
  }

  _Executor _M_executor;
};

template <class _T>
class __executor_binder_base_target
{
protected:
  template <class... _Args> explicit __executor_binder_base_target(_Args&&... __args)
    : _M_target(forward<_Args>(__args)...)
  {
  }

  _T _M_target;
};

} // inline namespace concurrency_v2
} // namespace experimental
} // namespace std

#endif
