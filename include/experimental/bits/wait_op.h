//
// wait_op.h
// ~~~~~~~~~
// Timer wait operation.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_WAIT_OP_H
#define EXECUTORS_EXPERIMENTAL_BITS_WAIT_OP_H

#include <system_error>
#include <experimental/executor>
#include <experimental/memory>
#include <experimental/bits/operation.h>
#include <experimental/bits/tuple_utils.h>

namespace std {
namespace experimental {
inline namespace concurrency_v1 {

class __wait_op_base
  : public __operation
{
protected:
  template <class _Clock, class _TimerTraits> friend class __timer_queue;
  error_code _M_ec;
};

template <class _Func>
class __wait_op
  : public __wait_op_base
{
public:
  template <class _F> explicit __wait_op(_F&& __f)
    : _M_func(forward<_F>(__f)), _M_work(get_associated_executor(_M_func))
  {
  }

  virtual void _Complete()
  {
    auto __allocator(get_associated_allocator(_M_func));
    auto __op(_Adopt_small_block(__allocator, this));
    executor_work<_Executor> __work(std::move(_M_work));
    _Executor __executor(__work.get_executor());
    auto __i(_Make_tuple_invoker(std::move(_M_func), _M_ec));
    __op.reset();
    __executor.post(std::move(__i), __allocator);
  }

  virtual void _Destroy()
  {
    auto __allocator(get_associated_allocator(_M_func));
    _Adopt_small_block(__allocator, this);
  }

private:
  _Func _M_func;
  typedef associated_executor_t<_Func> _Executor;
  executor_work<_Executor> _M_work;
};

} // inline namespace concurrency_v1
} // namespace experimental
} // namespace std

#endif
