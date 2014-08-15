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
#include <vector>
#include <experimental/bits/scheduler.h>

namespace std {
namespace experimental {
inline namespace concurrency_v1 {

class __system_executor_impl
  : public execution_context
{
public:
  __system_executor_impl()
  {
#if defined(_MSC_VER)
    auto sp = std::make_shared<int>(0);
    _M_token = sp;
#endif
    _M_scheduler._Work_started();
    std::size_t __n = thread::hardware_concurrency();
    for (size_t __i = 0; __i < __n; ++__i)
    {
#if defined(_MSC_VER)
      _M_threads.emplace_back([sp, this]() { _M_scheduler._Run(); });
#else
      _M_threads.emplace_back([this]() { _M_scheduler._Run(); });
#endif
    }
  }

  ~__system_executor_impl()
  {
    _M_scheduler._Work_finished();
    _M_scheduler._Stop();
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
    shutdown_context();
  }

  static __system_executor_impl& _Instance()
  {
    static __system_executor_impl __e;
    return __e;
  }

  template <class _F, class _A> void _Post(_F&& __f, const _A& __a)
  {
    _M_scheduler._Post(forward<_F>(__f), __a);
  }

  template <class _F, class _A> void _Defer(_F&& __f, const _A& __a)
  {
    _M_scheduler._Defer(forward<_F>(__f), __a);
  }

private:
  __scheduler _M_scheduler;
  vector<thread> _M_threads;
#if defined(_MSC_VER)
  std::weak_ptr<void> _M_token;
#endif
};

inline execution_context& system_executor::context() noexcept
{
  return __system_executor_impl::_Instance();
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
  std::move(tmp)();
}

template <class _Func, class _Alloc>
inline void system_executor::post(_Func&& __f, const _Alloc& __a)
{
  __system_executor_impl::_Instance()._Post(forward<_Func>(__f), __a);
}

template <class _Func, class _Alloc>
inline void system_executor::defer(_Func&& __f, const _Alloc& __a)
{
  __system_executor_impl::_Instance()._Defer(forward<_Func>(__f), __a);
}

inline bool operator==(const system_executor&, const system_executor&) noexcept
{
  return true;
}

inline bool operator!=(const system_executor&, const system_executor&) noexcept
{
  return false;
}

} // inline namespace concurrency_v1
} // namespace experimental
} // namespace std

#endif
