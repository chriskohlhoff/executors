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

template <class _Func, class _Work>
class __wait_invoker
{
public:
  template <class _F> __wait_invoker(_F&& __f,
    const _Work& __w, const error_code& __ec)
      : _M_func(forward<_F>(__f)), _M_work(__w), _M_ec(__ec)
  {
  }

  void operator()()
  {
    _M_func(_M_ec);
  }

private:
  _Func _M_func;
  _Work _M_work;
  const error_code _M_ec;
};

template <class _Func, class _Work>
class __wait_op
  : public __wait_op_base
{
public:
  template <class _F> __wait_op(_F&& __f, const _Work& __w)
    : _M_func(forward<_F>(__f)), _M_work(__w)
  {
  }

  virtual void _Complete()
  {
    unique_ptr<__wait_op> __op(this);
    get_executor(_M_work).post(
      __wait_invoker<_Func, _Work>(std::move(_M_func), _M_work, _M_ec));
  }

  virtual void _Destroy()
  {
    delete this;
  }

private:
  _Func _M_func;
  _Work _M_work;
};

} // namespace experimental
} // namespace std

#endif
