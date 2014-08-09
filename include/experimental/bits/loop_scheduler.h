//
// loop_scheduler.h
// ~~~~~~~~~~~~~~~~
// Loop scheduler implementation.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_LOOP_SCHEDULER_H
#define EXECUTORS_EXPERIMENTAL_BITS_LOOP_SCHEDULER_H

#include <cassert>

namespace std {
namespace experimental {
inline namespace concurrency_v1 {

inline loop_scheduler::loop_scheduler()
{
}

inline loop_scheduler::loop_scheduler(size_t __concurrency_hint)
  : __scheduler(__concurrency_hint)
{
}

inline loop_scheduler::~loop_scheduler()
{
}

inline loop_scheduler::executor_type loop_scheduler::get_executor() const noexcept
{
  return executor_type(const_cast<loop_scheduler*>(this));
}

inline size_t loop_scheduler::run()
{
  return _Run();
}

template <class _Rep, class _Period>
size_t loop_scheduler::run_for(
  const chrono::duration<_Rep, _Period>& __rel_time)
{
  return this->_Run_for(__rel_time);
}

template <class _Clock, class _Duration>
size_t loop_scheduler::run_until(
  const chrono::time_point<_Clock, _Duration>& __abs_time)
{
  return this->_Run_until(__abs_time);
}

inline size_t loop_scheduler::run_one()
{
  return _Run_one();
}

template <class _Rep, class _Period>
size_t loop_scheduler::run_one_for(
  const chrono::duration<_Rep, _Period>& __rel_time)
{
  return this->_Run_one_for(__rel_time);
}

template <class _Clock, class _Duration>
size_t loop_scheduler::run_one_until(
  const chrono::time_point<_Clock, _Duration>& __abs_time)
{
  return this->_Run_one_until(__abs_time);
}

inline size_t loop_scheduler::poll()
{
  return _Poll();
}

inline size_t loop_scheduler::poll_one()
{
  return _Poll_one();
}

inline void loop_scheduler::stop()
{
  _Stop();
}

inline bool loop_scheduler::stopped() const
{
  return _Stopped();
}

inline void loop_scheduler::restart()
{
  _Restart();
}

inline bool loop_scheduler::executor_type::running_in_this_thread() const noexcept
{
  return _M_scheduler->_Running_in_this_thread();
}

inline loop_scheduler& loop_scheduler::executor_type::context() noexcept
{
  return *_M_scheduler;
}

inline void loop_scheduler::executor_type::on_work_started() noexcept
{
  _M_scheduler->_Work_started();
}

inline void loop_scheduler::executor_type::on_work_finished() noexcept
{
  _M_scheduler->_Work_finished();
}

template <class _Func, class _Alloc>
void loop_scheduler::executor_type::dispatch(_Func&& __f, const _Alloc& __a)
{
  _M_scheduler->_Dispatch(forward<_Func>(__f), __a);
}

template <class _Func, class _Alloc>
void loop_scheduler::executor_type::post(_Func&& __f, const _Alloc& __a)
{
  _M_scheduler->_Post(forward<_Func>(__f), __a);
}

template <class _Func, class _Alloc>
void loop_scheduler::executor_type::defer(_Func&& __f, const _Alloc& __a)
{
  _M_scheduler->_Defer(forward<_Func>(__f), __a);
}

inline bool operator==(const loop_scheduler::executor_type& __a, const loop_scheduler::executor_type& __b) noexcept
{
  return __a._M_scheduler == __b._M_scheduler;
}

inline bool operator!=(const loop_scheduler::executor_type& __a, const loop_scheduler::executor_type& __b) noexcept
{
  return !(__a == __b);
}

} // inline namespace concurrency_v1
} // namespace experimental
} // namespace std

#endif
