//
// exception_ptr_executor.h
// ~~~~~~~~~~~~~~~~~~~~~~~~
// Executor used to capture exceptions into an execption_ptr.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_EXCEPTION_PTR_EXECUTOR_H
#define EXECUTORS_EXPERIMENTAL_BITS_EXCEPTION_PTR_EXECUTOR_H

#include <utility>

namespace std {
namespace experimental {
inline namespace concurrency_v1 {

template <class _Func>
struct __exception_ptr_call_wrapper
{
  exception_ptr* _M_exception;
  _Func _M_func;

  void operator()()
  {
    try
    {
      std::move(_M_func)();
    }
    catch (...)
    {
      *_M_exception = current_exception();
    }
  }
};

template <class _Executor>
struct __exception_ptr_executor
{
  exception_ptr* _M_exception;
  _Executor _M_executor;

  __exception_ptr_executor(exception_ptr* __ep, const _Executor& __ex)
    : _M_exception(__ep), _M_executor(__ex)
  {
  }

  execution_context& context() noexcept
  {
    return _M_executor.context();
  }

  void on_work_started() noexcept
  {
    _M_executor.on_work_started();
  }

  void on_work_finished() noexcept
  {
    _M_executor.on_work_finished();
  }

  template <class _F, class _A> void dispatch(_F&& __f, const _A& __a)
  {
    typedef typename decay<_F>::type _Func;
    _M_executor.dispatch(__exception_ptr_call_wrapper<_Func>{_M_exception, forward<_F>(__f)}, __a);
  }

  template <class _F, class _A> void post(_F&& __f, const _A& __a)
  {
    typedef typename decay<_F>::type _Func;
    _M_executor.post(__exception_ptr_call_wrapper<_Func>{_M_exception, forward<_F>(__f)}, __a);
  }

  template <class _F, class _A> void defer(_F&& __f, const _A& __a)
  {
    typedef typename decay<_F>::type _Func;
    _M_executor.defer(__exception_ptr_call_wrapper<_Func>{_M_exception, forward<_F>(__f)}, __a);
  }

  friend bool operator==(const __exception_ptr_executor& __a, const __exception_ptr_executor& __b) noexcept
  {
    return __a._M_exception == __b._M_exception;
  }

  friend bool operator!=(const __exception_ptr_executor& __a, const __exception_ptr_executor& __b) noexcept
  {
    return __a._M_exception != __b._M_exception;
  }
};

template <class _Executor>
struct is_executor<__exception_ptr_executor<_Executor>> : true_type {};

} // inline namespace concurrency_v1
} // namespace experimental
} // namespace std

#endif
