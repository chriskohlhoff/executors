//
// thread_pool.h
// ~~~~~~~~~~~~~
// Simple thread pool.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_THREAD_POOL_H
#define EXECUTORS_EXPERIMENTAL_BITS_THREAD_POOL_H

#include <stdexcept>

namespace std {
namespace experimental {
inline namespace concurrency_v1 {

inline thread_pool::thread_pool()
  : thread_pool(thread::hardware_concurrency())
{
}

inline thread_pool::thread_pool(size_t __num_threads)
  : __scheduler(__num_threads)
{
  if (__num_threads > 0)
  {
    _Work_started();
    for (size_t __i = 0; __i < __num_threads; ++__i)
      _M_threads.emplace_back([this](){ _Run(); });
  }
}

inline thread_pool::~thread_pool()
{
  _Stop();
  join();
}

inline thread_pool::executor_type thread_pool::get_executor() const noexcept
{
  return executor_type(const_cast<thread_pool*>(this));
}

inline void thread_pool::stop()
{
  _Stop();
}

inline void thread_pool::join()
{
  if (!_M_threads.empty())
  {
    _Work_finished();
    for (auto& __t: _M_threads)
      __t.join();
    _M_threads.clear();
  }
}

inline bool thread_pool::executor_type::running_in_this_thread() const noexcept
{
  return _M_pool->_Running_in_this_thread();
}

inline thread_pool& thread_pool::executor_type::context() noexcept
{
  return *_M_pool;
}

inline void thread_pool::executor_type::on_work_started() noexcept
{
  _M_pool->_Work_started();
}

inline void thread_pool::executor_type::on_work_finished() noexcept
{
  _M_pool->_Work_finished();
}

template <class _Func, class _Alloc>
void thread_pool::executor_type::dispatch(_Func&& __f, const _Alloc& __a)
{
  _M_pool->_Dispatch(forward<_Func>(__f), __a);
}

template <class _Func, class _Alloc>
void thread_pool::executor_type::post(_Func&& __f, const _Alloc& __a)
{
  _M_pool->_Post(forward<_Func>(__f), __a);
}

template <class _Func, class _Alloc>
void thread_pool::executor_type::defer(_Func&& __f, const _Alloc& __a)
{
  _M_pool->_Defer(forward<_Func>(__f), __a);
}

inline bool operator==(const thread_pool::executor_type& __a, const thread_pool::executor_type& __b) noexcept
{
  return __a._M_pool == __b._M_pool;
}

inline bool operator!=(const thread_pool::executor_type& __a, const thread_pool::executor_type& __b) noexcept
{
  return !(__a == __b);
}

} // inline namespace concurrency_v1
} // namespace experimental
} // namespace std

#endif
