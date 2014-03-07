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

inline loop_scheduler::loop_scheduler()
{
}

inline loop_scheduler::loop_scheduler(size_t __concurrency_hint)
{
  (void)__concurrency_hint;
}

inline loop_scheduler::~loop_scheduler()
{
}

inline size_t loop_scheduler::run()
{
  return _Run();
}

inline size_t loop_scheduler::run_one()
{
  return _Run_one();
}

template <class _Rep, class _Period>
size_t loop_scheduler::run_for(
  const chrono::duration<_Rep, _Period>& __rel_time)
{
  assert(0 && "not implemented yet");
  return 0;
}

template <class _Clock, class _Duration>
size_t loop_scheduler::run_until(
  const chrono::time_point<_Clock, _Duration>& __abs_time)
{
  assert(0 && "not implemented yet");
  return 0;
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

inline void loop_scheduler::reset()
{
  _Reset();
}

inline loop_scheduler::executor::executor(
  const loop_scheduler::executor& __e)
    : _M_scheduler(__e._M_scheduler)
{
}

inline loop_scheduler::executor&
  loop_scheduler::executor::operator=(const executor& __e)
{
  _M_scheduler = __e._M_scheduler;
  return *this;
}

inline loop_scheduler::executor::~executor()
{
}

template <class _Func> void loop_scheduler::executor::post(_Func&& __f)
{
  _M_scheduler->_Post(forward<_Func>(__f));
}

template <class _Func> void loop_scheduler::executor::dispatch(_Func&& __f)
{
  _M_scheduler->_Dispatch(forward<_Func>(__f));
}

inline loop_scheduler::executor::work loop_scheduler::executor::make_work()
{
  return work(_M_scheduler);
}

inline execution_context& loop_scheduler::executor::context()
{
  return *_M_scheduler;
}

inline loop_scheduler::executor::work::work(loop_scheduler* __s)
  : _M_scheduler(__s)
{
  _M_scheduler->_Work_started();
}

inline loop_scheduler::executor::work::work(
  const loop_scheduler::executor::work& __w)
    : _M_scheduler(__w._M_scheduler)
{
  _M_scheduler->_Work_started();
}

inline loop_scheduler::executor::work&
  loop_scheduler::executor::work::operator=(const work& __w)
{
  loop_scheduler* __tmp = _M_scheduler;
  _M_scheduler = __w._M_scheduler;
  _M_scheduler->_Work_started();
  __tmp->_Work_finished();
  return *this;
}

inline loop_scheduler::executor::work::~work()
{
  _M_scheduler->_Work_finished();
}

inline loop_scheduler::executor get_executor(loop_scheduler& __s)
{
  return loop_scheduler::executor(&__s);
}

inline loop_scheduler::executor get_executor(const loop_scheduler::executor& __e)
{
  return __e;
}

inline loop_scheduler::executor get_executor(loop_scheduler::executor&& __e)
{
  return std::move(__e);
}

inline loop_scheduler::executor
  get_executor(const loop_scheduler::executor::work& __w)
{
  return loop_scheduler::executor(__w._M_scheduler);
}

inline loop_scheduler::executor
  get_executor(loop_scheduler::executor::work&& __w)
{
  return loop_scheduler::executor(__w._M_scheduler);
}

} // namespace experimental
} // namespace std

#endif
