//
// timer_op.h
// ~~~~~~~~~~
// Timer wait operation.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_TIMER_OP_H
#define EXECUTORS_EXPERIMENTAL_BITS_TIMER_OP_H

#include <experimental/executor>
#include <experimental/memory>
#include <experimental/bits/operation.h>
#include <experimental/bits/reactor.h>
#include <experimental/bits/timer_queue.h>
#include <experimental/bits/timer_service.h>

namespace std {
namespace experimental {
inline namespace concurrency_v1 {

template <class _Clock, class _Executor, class _Action, class _Handler>
class __timer_op
  : public __operation
{
public:
  __timer_op(const _Executor& __e, _Handler& __h)
    : _M_handler(std::move(__h)), _M_work_1(__e),
      _M_work_2((get_associated_executor)(_M_handler))
  {
  }

  static void _Enqueue(const _Executor& __e, const typename _Clock::time_point& __expiry, _Handler& __h)
  {
    __timer_service<_Clock>& __svc = use_service<__timer_service<_Clock>>(_Executor(__e).context());
    auto __allocator((get_associated_allocator)(__h));
    auto __op(_Allocate_small_block<__timer_op>(__allocator, __e, __h));
    __svc._M_reactor._Enqueue_timer(__svc._M_queue, __expiry, __op.get()->_M_data, __op.get());
    __op.release();
  }

  virtual void _Complete()
  {
    auto __allocator((get_associated_allocator)(_M_handler));
    auto __op(_Adopt_small_block(__allocator, this));
    _Handler __handler(std::move(_M_handler));
    executor_work<_Executor> __work_1(std::move(_M_work_1));
    executor_work<associated_executor_t<_Handler>> __work_2(std::move(_M_work_2));
    _Executor __executor(__work_1.get_executor());
    __op.reset();
    _Action::_Perform(__executor, std::move(__handler), __allocator);
  }

  virtual void _Destroy()
  {
    auto __allocator(get_associated_allocator(_M_handler));
    _Adopt_small_block(__allocator, this);
  }

private:
  _Handler _M_handler;
  executor_work<_Executor> _M_work_1;
  executor_work<associated_executor_t<_Handler>> _M_work_2;
  typename __timer_queue<_Clock>::__per_timer_data _M_data;
};

} // inline namespace concurrency_v1
} // namespace experimental
} // namespace std

#endif
