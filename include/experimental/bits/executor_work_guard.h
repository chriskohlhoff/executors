//
// executor_work_guard.h
// ~~~~~~~~~~~~~~~~~~~~~
// Controls ownership of executor work within a scope.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_EXECUTOR_WORK_GUARD_H
#define EXECUTORS_EXPERIMENTAL_BITS_EXECUTOR_WORK_GUARD_H

#include <type_traits>

namespace std {
namespace experimental {
inline namespace concurrency_v2 {

template <class _Executor>
inline executor_work_guard<_Executor>::executor_work_guard(const executor_type& __e) noexcept
  : _M_executor(__e), _M_owns(true)
{
  _M_executor.on_work_started();
}

template <class _Executor>
inline executor_work_guard<_Executor>::executor_work_guard(const executor_work_guard& __w) noexcept
  : _M_executor(__w._M_executor), _M_owns(__w._M_owns)
{
  if (_M_owns)
    _M_executor.on_work_started();
}

template <class _Executor>
inline executor_work_guard<_Executor>::executor_work_guard(executor_work_guard&& __w) noexcept
  : _M_executor(std::move(__w._M_executor)),
    _M_owns(__w._M_owns)
{
  __w._M_owns = false;
}

template <class _Executor>
inline executor_work_guard<_Executor>::~executor_work_guard()
{
  if (_M_owns)
    _M_executor.on_work_finished();
}

template <class _Executor>
inline typename executor_work_guard<_Executor>::executor_type
executor_work_guard<_Executor>::get_executor() const noexcept
{
  return _M_executor;
}

template <class _Executor>
inline bool executor_work_guard<_Executor>::owns_work() const noexcept
{
  return _M_owns;
}

template <class _Executor>
inline void executor_work_guard<_Executor>::reset() noexcept
{
  if (_M_owns)
  {
    _M_executor.on_work_finished();
    _M_owns = false;
  }
}

} // inline namespace concurrency_v2
} // namespace experimental
} // namespace std

#endif
