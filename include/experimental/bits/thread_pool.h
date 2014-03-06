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

namespace std {
namespace experimental {

inline thread_pool::thread_pool()
  : thread_pool(thread::hardware_concurrency())
{
}

inline thread_pool::thread_pool(size_t __num_threads)
{
  _M_scheduler._Work_started();
  for (size_t __i = 0; __i < __num_threads; ++__i)
    _M_threads.emplace_back([this](){ _M_scheduler._Run(); });
}

inline thread_pool::~thread_pool()
{
  _M_scheduler._Work_finished();
  for (auto& __t: _M_threads)
    __t.join();
}

inline thread_pool::executor thread_pool::get_executor()
{
  return executor(&_M_scheduler);
}

inline thread_pool::executor::executor(
  const thread_pool::executor& __e)
    : _M_scheduler(__e._M_scheduler)
{
}

inline thread_pool::executor&
  thread_pool::executor::operator=(const executor& __e)
{
  _M_scheduler = __e._M_scheduler;
  return *this;
}

inline thread_pool::executor::~executor()
{
}

template <class _Func> void thread_pool::executor::post(_Func&& __f)
{
  _M_scheduler->_Post(forward<_Func>(__f));
}

template <class _Func> void thread_pool::executor::dispatch(_Func&& __f)
{
  _M_scheduler->_Dispatch(forward<_Func>(__f));
}

inline thread_pool::executor::work thread_pool::executor::make_work()
{
  return work(_M_scheduler);
}

inline thread_pool::executor::work::work(__scheduler* __s)
  : _M_scheduler(__s)
{
  _M_scheduler->_Work_started();
}

inline thread_pool::executor::work::work(
  const thread_pool::executor::work& __w)
    : _M_scheduler(__w._M_scheduler)
{
  _M_scheduler->_Work_started();
}

inline thread_pool::executor::work&
  thread_pool::executor::work::operator=(const work& __w)
{
  __scheduler* __tmp = _M_scheduler;
  _M_scheduler = __w._M_scheduler;
  _M_scheduler->_Work_started();
  __tmp->_Work_finished();
  return *this;
}

inline thread_pool::executor::work::~work()
{
  _M_scheduler->_Work_finished();
}

} // namespace experimental
} // namespace std

#endif
