//
// strand.h
// ~~~~~~~~
// Strand used for preventing non-concurrent invocation of handlers.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_STRAND_H
#define EXECUTORS_EXPERIMENTAL_BITS_STRAND_H

#include <atomic>
#include <mutex>
#include <experimental/type_traits>
#include <experimental/bits/call_stack.h>
#include <experimental/bits/operation.h>
#include <experimental/bits/small_block_recycler.h>

namespace std {
namespace experimental {

class __strand_service;

struct __strand_impl
{
  explicit __strand_impl(__strand_service& __s);
  __strand_impl(const __strand_impl&) = delete;
  __strand_impl& operator=(const __strand_impl&) = delete;
  ~__strand_impl();

  atomic_flag _M_lock = ATOMIC_FLAG_INIT;
  bool _M_ready = false;
  __op_queue<__operation> _M_waiting_queue;
  __op_queue<__operation> _M_ready_queue;
  __strand_service* _M_service = nullptr;
  __strand_impl* _M_prev = nullptr;
  __strand_impl* _M_next = nullptr;

  class _Lock_guard
  {
  public:
    _Lock_guard(const _Lock_guard&) = delete;
    _Lock_guard& operator=(const _Lock_guard&) = delete;

    explicit _Lock_guard(__strand_impl& __i) : _M_lock(__i._M_lock)
    {
      while (_M_lock.test_and_set(memory_order_acquire)) {}
    }

    ~_Lock_guard()
    {
      _M_lock.clear(memory_order_release);
    }

  private:
    atomic_flag& _M_lock;
  };
};

class __strand_service
  : public execution_context::service
{
public:
  __strand_service(execution_context& __c)
    : execution_context::service(__c), _M_first(nullptr)
  {
  }

  ~__strand_service()
  {
  }

  void shutdown_service()
  {
    __op_queue<__operation> __ops;
    lock_guard<mutex> __lock(_M_mutex);

    for (__strand_impl* __i = _M_first; __i; __i = __i->_M_next)
    {
      __strand_impl::_Lock_guard __strand_lock(*__i);
      __ops._Push(__i->_M_waiting_queue);
      __ops._Push(__i->_M_ready_queue);
    }
  }

private:
  template <class> friend class strand;
  friend struct __strand_impl;
  mutable mutex _M_mutex;
  __strand_impl* _M_first;
};

__strand_impl::__strand_impl(__strand_service& __s)
  : _M_service(&__s)
{
  lock_guard<mutex> __lock(_M_service->_M_mutex);

  _M_next = _M_service->_M_first;
  if (_M_service->_M_first)
    _M_service->_M_first->_M_prev = this;
  _M_service->_M_first = this;
}

__strand_impl::~__strand_impl()
{
  lock_guard<mutex> __lock(_M_service->_M_mutex);

  if (_M_service->_M_first == this)
    _M_service->_M_first = _M_next;
  if (_M_prev)
    _M_prev->_M_next = _M_next;
  if (_M_next)
    _M_next->_M_prev = _M_prev;
}

template <class _Executor> template <class _Dummy>
inline strand<_Executor>::strand(_Dummy, typename enable_if<is_default_constructible<_Executor>::value, _Dummy>::type*)
  : _M_executor(),
    _M_impl(make_shared<__strand_impl>(use_service<__strand_service>(_M_executor.context())))
{
}

template <class _Executor>
inline strand<_Executor>::strand(_Executor __e)
  : _M_executor(std::move(__e)),
    _M_impl(make_shared<__strand_impl>(use_service<__strand_service>(_M_executor.context())))
{
}

template <class _Executor>
inline strand<_Executor>::strand(const strand& __s)
  : _M_executor(__s._M_executor), _M_impl(__s._M_impl)
{
}

template <class _Executor>
inline strand<_Executor>::strand(strand&& __s)
  : _M_executor(std::move(__s._M_executor)), _M_impl(std::move(__s._M_impl))
{
}

template <class _Executor> template <class _OtherExecutor>
inline strand<_Executor>::strand(const strand<_OtherExecutor>& __s)
  : _M_executor(__s._M_executor), _M_impl(__s._M_impl)
{
}

template <class _Executor> template <class _OtherExecutor>
inline strand<_Executor>::strand(strand<_OtherExecutor>&& __s)
  : _M_executor(std::move(__s._M_executor)), _M_impl(std::move(__s._M_impl))
{
}

template <class _Executor>
inline strand<_Executor>::strand(_Executor&& __e, const shared_ptr<__strand_impl>& __i)
  : _M_executor(forward<_Executor>(__e)), _M_impl(__i)
{
}

template <class _Executor>
inline strand<_Executor>& strand<_Executor>::operator=(const strand& __s)
{
  _M_executor = __s._M_executor;
  _M_impl = __s._M_impl;
  return *this;
}

template <class _Executor>
inline strand<_Executor>& strand<_Executor>::operator=(strand&& __s)
{
  _M_executor = std::move(__s._M_executor);
  _M_impl = std::move(__s._M_impl);
  return *this;
}

template <class _Executor> template <class _OtherExecutor>
inline strand<_Executor>& strand<_Executor>::operator=(const strand<_OtherExecutor>& __s)
{
  _M_executor = __s._M_executor;
  _M_impl = __s._M_impl;
  return *this;
}

template <class _Executor> template <class _OtherExecutor>
inline strand<_Executor>& strand<_Executor>::operator=(strand<_OtherExecutor>&& __s)
{
  _M_executor = std::move(__s._M_executor);
  _M_impl = std::move(__s._M_impl);
  return *this;
}

template <class _Executor>
inline strand<_Executor>::~strand()
{
}

template <class _Func>
class __strand_op
  : public __operation
{
public:
  __strand_op(const __strand_op&) = delete;
  __strand_op& operator=(const __strand_op&) = delete;

  template <class _F> __strand_op(_F&& __f)
    : _M_func(forward<_F>(__f))
  {
  }

  __strand_op(__strand_op&& __s)
    : _M_func(std::move(__s._M_func))
  {
  }

  virtual void _Complete()
  {
    __small_block_recycler<>::_Unique_ptr<__strand_op> __op(this);
    _Func __tmp(std::move(_M_func));
    __op.reset();
    __tmp();
  }

  virtual void _Destroy()
  {
    __small_block_recycler<>::_Destroy(this);
  }

private:
  _Func _M_func;
};

template <class _Executor>
struct __strand_invoker
{
  typename _Executor::work _M_work;
  shared_ptr<__strand_impl> _M_impl;

  struct _On_exit
  {
    __strand_invoker* _M_invoker;

    ~_On_exit()
    {
      bool __more;
      {
        __strand_impl::_Lock_guard __lock(*_M_invoker->_M_impl);
        _M_invoker->_M_impl->_M_ready_queue._Push(_M_invoker->_M_impl->_M_waiting_queue);
        __more = _M_invoker->_M_impl->_M_ready = !_M_invoker->_M_impl->_M_ready_queue._Empty();
      }

      if (__more)
      {
        auto __executor(make_executor(_M_invoker->_M_work));
        __executor.post(std::move(*_M_invoker));
      }
    }
  };

  void operator()()
  {
    __call_stack<__strand_impl>::__context __ctx(_M_impl.get());

    _On_exit __on_exit{this};
    (void)__on_exit;

    while (__operation* __op = _M_impl->_M_ready_queue._Front())
    {
      _M_impl->_M_ready_queue._Pop();
      __op->_Complete();
    }
  }
};

template <class _Executor> template <class _Func>
void strand<_Executor>::post(_Func&& __f)
{
  typedef typename decay<_Func>::type _DecayFunc;
  __small_block_recycler<>::_Unique_ptr<__strand_op<_DecayFunc>> __op(
    __small_block_recycler<>::_Create<__strand_op<_DecayFunc>>(forward<_Func>(__f)));

  {
    __strand_impl::_Lock_guard __lock(*_M_impl);

    if (_M_impl->_M_ready)
    {
      _M_impl->_M_waiting_queue._Push(__op.get());
      __op.release();
      return;
    }

    _M_impl->_M_ready = true;
  }

  _M_impl->_M_ready_queue._Push(__op.get());
  __op.release();

  _M_executor.post(__strand_invoker<_Executor>{_M_executor.make_work(), _M_impl});
}

template <class _Executor> template <class _Func>
void strand<_Executor>::dispatch(_Func&& __f)
{
  typedef typename decay<_Func>::type _DecayFunc;
  if (__call_stack<__strand_impl>::_Contains(_M_impl.get()))
  {
    _DecayFunc __tmp(forward<_Func>(__f));
    __tmp();
    return;
  }

  typedef typename decay<_Func>::type _DecayFunc;
  __small_block_recycler<>::_Unique_ptr<__strand_op<_DecayFunc>> __op(
    __small_block_recycler<>::_Create<__strand_op<_DecayFunc>>(forward<_Func>(__f)));

  {
    __strand_impl::_Lock_guard __lock(*_M_impl);

    if (_M_impl->_M_ready)
    {
      _M_impl->_M_waiting_queue._Push(__op.get());
      __op.release();
      return;
    }

    _M_impl->_M_ready = true;
  }

  _M_impl->_M_ready_queue._Push(__op.get());
  __op.release();

  _M_executor.dispatch(__strand_invoker<_Executor>{_M_executor.make_work(), _M_impl});
}

template <class _Executor>
inline typename strand<_Executor>::work strand<_Executor>::make_work()
{
  return work(_M_executor.make_work(), _M_impl);
}

template <class _Executor> template <class _Func>
inline auto strand<_Executor>::wrap(_Func&& __f)
{
  return (wrap_with_executor)(forward<_Func>(__f), *this);
}

template <class _Executor>
inline execution_context& strand<_Executor>::context()
{
  return _M_executor.context();
}

template <class _Executor>
inline strand<_Executor>::work::work(const work& __w)
  : _M_work(__w._M_work), _M_impl(__w._M_impl)
{
}

template <class _Executor>
inline strand<_Executor>::work::work(work&& __w)
  : _M_work(__w._M_work), _M_impl(__w._M_impl)
{
}

template <class _Executor>
inline strand<_Executor>::work::work(typename _Executor::work&& __w, const shared_ptr<__strand_impl>& __i)
  : _M_work(forward<typename _Executor::work>(__w)), _M_impl(__i)
{
}

template <class _Executor>
inline typename strand<_Executor>::work& strand<_Executor>::work::operator=(const work& __w)
{
  _M_work = __w._M_work;
  _M_impl = __w._M_impl;
  return *this;
}

template <class _Executor>
inline typename strand<_Executor>::work& strand<_Executor>::work::operator=(work&& __w)
{
  _M_work = std::move(__w._M_work);
  _M_impl = std::move(__w._M_impl);
  return *this;
}

template <class _Executor>
inline strand<_Executor>::work::~work()
{
}

template <class _Executor>
inline strand<_Executor> make_executor(const strand<_Executor>& __e)
{
  return __e;
}

template <class _Executor>
inline strand<_Executor> make_executor(strand<_Executor>&& __e)
{
  return std::move(__e);
}

template <class _T> inline auto make_strand(_T&& __t)
{
  return strand<typename decay<_T>::type>(forward<_T>(__t));
}

} // namespace experimental
} // namespace std

#endif
