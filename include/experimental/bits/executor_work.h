//
// executor_work.h
// ~~~~~~~~~~~~~~~
// Controls ownership of executor work within a scope.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_EXECUTOR_WORK_H
#define EXECUTORS_EXPERIMENTAL_BITS_EXECUTOR_WORK_H

#include <type_traits>
#include <experimental/bits/function_traits.h>

namespace std {
namespace experimental {
inline namespace concurrency_v1 {

template <class _Executor>
inline executor_work<_Executor>::executor_work(const executor_type& __e) noexcept
  : _M_executor(__e), _M_owns(true)
{
  _M_executor.on_work_started();
}

template <class _Executor>
inline executor_work<_Executor>::executor_work(const executor_work& __w) noexcept
  : _M_executor(__w._M_executor), _M_owns(__w._M_owns)
{
  if (_M_owns)
    _M_executor.on_work_started();
}

template <class _Executor>
inline executor_work<_Executor>::executor_work(executor_work&& __w) noexcept
  : _M_executor(std::move(__w._M_executor)),
    _M_owns(__w._M_owns)
{
  __w._M_owns = false;
}

template <class _Executor>
inline executor_work<_Executor>::~executor_work()
{
  if (_M_owns)
    _M_executor.on_work_finished();
}

template <class _Executor>
inline typename executor_work<_Executor>::executor_type
executor_work<_Executor>::get_executor() const noexcept
{
  return _M_executor;
}

template <class _Executor>
inline bool executor_work<_Executor>::owns_work() const noexcept
{
  return _M_owns;
}

template <class _Executor>
inline void executor_work<_Executor>::reset() noexcept
{
  if (_M_owns)
  {
    _M_executor.on_work_finished();
    _M_owns = false;
  }
}

} // inline namespace concurrency_v1
} // namespace experimental
} // namespace std

#endif
