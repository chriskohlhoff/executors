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
#include <type_traits>
#include <thread>
#include <vector>
#include <experimental/bits/scheduler.h>
#include <experimental/bits/wrapper.h>

namespace std {
namespace experimental {

class __system_executor_impl
  : public execution_context
{
public:
  __system_executor_impl()
  {
    _M_scheduler._Work_started();
    std::size_t __n = thread::hardware_concurrency();
    for (size_t __i = 0; __i < __n; ++__i)
      _M_threads.emplace_back([this](){ _M_scheduler._Run(); });
  }

  ~__system_executor_impl()
  {
    shutdown();
    _M_scheduler._Work_finished();
    for (auto& __t: _M_threads)
      __t.join();
  }

  static __system_executor_impl& _Instance()
  {
    static __system_executor_impl __e;
    return __e;
  }

  template <class _F> void _Post(_F&& __f)
  {
    _M_scheduler._Post(forward<_F>(__f));
  }

private:
  __scheduler _M_scheduler;
  vector<thread> _M_threads;
};

template <class _Func>
inline void system_executor::post(_Func&& __f)
{
  __system_executor_impl::_Instance()._Post(forward<_Func>(__f));
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

template <class _Func>
inline auto system_executor::wrap(_Func&& __f)
{
  return __wrapper<typename decay<_Func>::type, system_executor>(forward<_Func>(__f), *this);
}

inline execution_context& system_executor::context()
{
  return __system_executor_impl::_Instance();
}

template <class _T>
inline system_executor get_executor(_T&&)
{
  return system_executor();
}

inline system_executor get_executor(const system_executor&)
{
  return system_executor();
}

inline system_executor get_executor(system_executor&&)
{
  return system_executor();
}

inline system_executor get_executor(const system_executor::work&)
{
  return system_executor();
}

inline system_executor get_executor(system_executor::work&&)
{
  return system_executor();
}

} // namespace experimental
} // namespace std

#endif
