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
  : __scheduler(__num_threads)
{
  _Work_started();
  for (size_t __i = 0; __i < __num_threads; ++__i)
    _M_threads.emplace_back([this](){ _Run(); });
}

inline thread_pool::~thread_pool()
{
  shutdown();
  _Work_finished();
  for (auto& __t: _M_threads)
    __t.join();
}

inline thread_pool::executor::executor(
  const thread_pool::executor& __e)
    : _M_pool(__e._M_pool)
{
}

inline thread_pool::executor&
  thread_pool::executor::operator=(const executor& __e)
{
  _M_pool = __e._M_pool;
  return *this;
}

inline thread_pool::executor::~executor()
{
}

template <class _Func> void thread_pool::executor::post(_Func&& __f)
{
  _M_pool->_Post(forward<_Func>(__f));
}

template <class _Func> void thread_pool::executor::dispatch(_Func&& __f)
{
  _M_pool->_Dispatch(forward<_Func>(__f));
}

inline thread_pool::executor::work thread_pool::executor::make_work()
{
  return work(_M_pool);
}

template <class _Func>
inline auto thread_pool::executor::wrap(_Func&& __f)
{
  return (wrap_with_executor)(forward<_Func>(__f), *this);
}

inline execution_context& thread_pool::executor::context()
{
  return *_M_pool;
}

inline thread_pool::executor::work::work(thread_pool* __p)
  : _M_pool(__p)
{
  _M_pool->_Work_started();
}

inline thread_pool::executor::work::work(
  const thread_pool::executor::work& __w)
    : _M_pool(__w._M_pool)
{
  _M_pool->_Work_started();
}

inline thread_pool::executor::work&
  thread_pool::executor::work::operator=(const work& __w)
{
  thread_pool* __tmp = _M_pool;
  _M_pool = __w._M_pool;
  _M_pool->_Work_started();
  __tmp->_Work_finished();
  return *this;
}

inline thread_pool::executor::work::~work()
{
  _M_pool->_Work_finished();
}

inline thread_pool::executor make_executor(thread_pool& __p)
{
  return thread_pool::executor(&__p);
}

inline thread_pool::executor make_executor(const thread_pool::executor& __e)
{
  return __e;
}

inline thread_pool::executor make_executor(thread_pool::executor&& __e)
{
  return std::move(__e);
}

inline thread_pool::executor make_executor(const thread_pool::executor::work& __w)
{
  return thread_pool::executor(__w._M_pool);
}

inline thread_pool::executor make_executor(thread_pool::executor::work&& __w)
{
  return thread_pool::executor(__w._M_pool);
}

} // namespace experimental
} // namespace std

#endif
