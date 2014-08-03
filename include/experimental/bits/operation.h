//
// operation.h
// ~~~~~~~~~~~
// Base class and queue for all pending operations.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_OPERATION_H
#define EXECUTORS_EXPERIMENTAL_BITS_OPERATION_H

namespace std {
namespace experimental {
inline namespace concurrency_v1 {

template <class _Operation> class __op_queue;

class __operation
{
public:
  virtual void _Destroy() = 0;
  virtual void _Complete() = 0;

protected:
  __operation() : _M_next(0) {}
  virtual ~__operation() {}

private:
  __operation* _M_next;
  template <class _Operation> friend class __op_queue;
};

template <class _Operation>
class __op_queue
{
public:
  __op_queue() : _M_front(0), _M_back(0) {}
  __op_queue(const __op_queue&) = delete;
  __op_queue& operator=(const __op_queue&) = delete;

  ~__op_queue()
  {
    while (_Operation* __op = _M_front)
    {
      _Pop();
      __op->_Destroy();
    }
  }

  _Operation* _Front() { return _M_front; }
  bool _Empty() const { return _M_front == 0; }

  void _Pop()
  {
    if (_Operation* __tmp = _M_front)
    {
      _M_front = static_cast<_Operation*>(__tmp->_M_next);
      if (_M_front == 0)
        _M_back = 0;
      __tmp->_M_next = 0;
    }
  }

  void _Push(_Operation* __op)
  {
    __op->_M_next = 0;
    if (_M_back)
    {
      _M_back->_M_next = __op;
      _M_back = __op;
    }
    else
    {
      _M_front = _M_back = __op;
    }
  }

  template <class _OtherOperation>
  void _Push(__op_queue<_OtherOperation>& __q)
  {
    if (_Operation* __other_front = __q._M_front)
    {
      if (_M_back)
        _M_back->_M_next = __other_front;
      else
        _M_front = __other_front;
      _M_back = __q._M_back;
      __q._M_front = __q._M_back = 0;
    }
  }

private:
  _Operation* _M_front;
  _Operation* _M_back;
  template <class _OtherOperation> friend class __op_queue;
};

} // inline namespace concurrency_v1
} // namespace experimental
} // namespace std

#endif
