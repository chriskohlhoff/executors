//
// function_op.h
// ~~~~~~~~~~~~~
// Function wrapped as an operation.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_FUNCTION_OP_H
#define EXECUTORS_EXPERIMENTAL_BITS_FUNCTION_OP_H

#include <experimental/bits/operation.h>
#include <experimental/bits/small_block_recycler.h>

namespace std {
namespace experimental {
inline namespace concurrency_v2 {

template <class _Func, class _Allocator>
class __function_op
  : public __operation
{
public:
  __function_op(const __function_op&) = delete;
  __function_op& operator=(const __function_op&) = delete;

  template <class _F> __function_op(_F&& __f, const _Allocator& __a)
    : _M_func(forward<_F>(__f)), _M_allocator(__a)
  {
  }

  __function_op(__function_op&& __s)
    : _M_func(std::move(__s._M_func)), _M_allocator(std::move(__s._M_allocator))
  {
  }

  ~__function_op()
  {
  }

  virtual void _Complete()
  {
    auto __op(_Adopt_small_block(_M_allocator, this));
    _Func __tmp(std::move(_M_func));
    __op.reset();
    __tmp();
  }

  virtual void _Destroy()
  {
    _Adopt_small_block(_M_allocator, this);
  }

private:
  _Func _M_func;
  _Allocator _M_allocator;
};

class __function_ptr
{
public:
  template <class _F, class _Alloc> __function_ptr(_F __f, const _Alloc& __a)
    : _M_func(_Allocate_small_block<__function_op<_F, _Alloc>>(__a, std::move(__f), __a).release())
  {
  }

  __function_ptr(const __function_ptr&) = delete;

  __function_ptr(__function_ptr&& __f)
    : _M_func(__f._M_func)
  {
    __f._M_func = nullptr;
  }

  ~__function_ptr()
  {
    if (_M_func)
      _M_func->_Destroy();
  }

  void operator()()
  {
    __operation* __f = _M_func;
    _M_func = nullptr;
    __f->_Complete();
  }

private:
  __operation* _M_func;
};

} // inline namespace concurrency_v2
} // namespace experimental
} // namespace std

#endif
