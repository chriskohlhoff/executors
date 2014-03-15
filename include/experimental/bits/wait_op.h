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

#include <memory>
#include <system_error>
#include <experimental/executor>
#include <experimental/bits/invoker.h>
#include <experimental/bits/operation.h>

namespace std {
namespace experimental {

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
    : _M_func(forward<_F>(__f)), _M_work(get_executor(_M_func).make_work())
  {
  }

  virtual void _Complete()
  {
    unique_ptr<__wait_op> __op(this);
    get_executor(_M_work).post(
      __invoke_with_result<const error_code, _Func>{_M_ec, std::move(_M_func)});
  }

  virtual void _Destroy()
  {
    delete this;
  }

private:
  _Func _M_func;
  typename decltype(get_executor(declval<_Func>()))::work _M_work;
};

} // namespace experimental
} // namespace std

#endif
