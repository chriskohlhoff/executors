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
#include <cstddef>
#include <memory>
#include <type_traits>
#include <thread>
#include <experimental/bits/scheduler.h>

namespace std {
namespace experimental {
inline namespace concurrency_v2 {

inline system_context::system_context(int)
{
#if defined(_MSC_VER)
  auto sp = std::make_shared<int>(0);
  _M_token = sp;
#endif
  _Work_started();
  std::size_t __n = thread::hardware_concurrency();
  for (size_t __i = 0; __i < __n; ++__i)
  {
#if defined(_MSC_VER)
    _M_threads.emplace_back([sp, this]() { _Run(); });
#else
    _M_threads.emplace_back([this]() { _Run(); });
#endif
  }
}

inline system_context::~system_context()
{
  _Work_finished();
  _Stop();
  join();
  shutdown();
}

inline system_context::executor_type system_context::get_executor() noexcept
{
  return system_executor();
}

inline void system_context::stop()
{
  return _Stop();
}

inline bool system_context::stopped() const noexcept
{
  return _Stopped();
}

inline void system_context::join()
{
#if defined(_MSC_VER)
  // Work around MSVC deadlock bug when joining threads in global destructors.
  while (!_M_token.expired())
    std::this_thread::yield();
  for (auto& __t: _M_threads)
    __t.detach();
#else
  for (auto& __t: _M_threads)
    __t.join();
#endif
}

inline system_context& system_context::_Instance()
{
  static system_context __e(0);
  return __e;
}

inline system_context& system_executor::context() noexcept
{
  return system_context::_Instance();
}

inline void system_executor::on_work_started() noexcept
{
  // No-op.
}

inline void system_executor::on_work_finished() noexcept
{
  // No-op.
}

template <class _Func, class _Alloc>
void system_executor::dispatch(_Func&& __f, const _Alloc&)
{
  typename decay<_Func>::type tmp(forward<_Func>(__f));
  tmp();
}

template <class _Func, class _Alloc>
inline void system_executor::post(_Func&& __f, const _Alloc& __a)
{
  system_context::_Instance()._Post(forward<_Func>(__f), __a);
}

template <class _Func, class _Alloc>
inline void system_executor::defer(_Func&& __f, const _Alloc& __a)
{
  system_context::_Instance()._Defer(forward<_Func>(__f), __a);
}

inline bool operator==(const system_executor&, const system_executor&) noexcept
{
  return true;
}

inline bool operator!=(const system_executor&, const system_executor&) noexcept
{
  return false;
}

} // inline namespace concurrency_v2
} // namespace experimental
} // namespace std

#endif
