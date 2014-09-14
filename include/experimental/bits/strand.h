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
inline namespace concurrency_v1 {

class __strand_service;

struct __strand_impl
{
  explicit __strand_impl(__strand_service& __s);
  __strand_impl(const __strand_impl&) = delete;
  __strand_impl& operator=(const __strand_impl&) = delete;
  ~__strand_impl();

#if defined(_MSC_VER)
  atomic_flag _M_lock;
#else
  atomic_flag _M_lock = ATOMIC_FLAG_INIT;
#endif
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

inline __strand_impl::__strand_impl(__strand_service& __s)
  : _M_service(&__s)
{
#if defined(_MSC_VER)
  _M_lock.clear();
#endif

  lock_guard<mutex> __lock(_M_service->_M_mutex);

  _M_next = _M_service->_M_first;
  if (_M_service->_M_first)
    _M_service->_M_first->_M_prev = this;
  _M_service->_M_first = this;
}

inline __strand_impl::~__strand_impl()
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
inline strand<_Executor>::strand(const _Executor& __e, const shared_ptr<__strand_impl>& __i)
  : _M_executor(__e), _M_impl(__i)
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

template <class _Executor>
inline typename strand<_Executor>::inner_executor_type strand<_Executor>::get_inner_executor() const noexcept
{
  return _M_executor;
}

template <class _Executor>
inline bool strand<_Executor>::running_in_this_thread() const noexcept
{
  return __call_stack<__strand_impl>::_Contains(_M_impl.get());
}

template <class _Executor>
inline execution_context& strand<_Executor>::context() noexcept
{
  return _M_executor.context();
}

template <class _Executor>
inline void strand<_Executor>::on_work_started() noexcept
{
  return _M_executor.on_work_started();
}

template <class _Executor>
inline void strand<_Executor>::on_work_finished() noexcept
{
  return _M_executor.on_work_finished();
}

template <class _Func, class _Alloc>
class __strand_op
  : public __operation
{
public:
  __strand_op(const __strand_op&) = delete;
  __strand_op& operator=(const __strand_op&) = delete;

  template <class _F> __strand_op(_F&& __f, const _Alloc& __a)
    : _M_func(forward<_F>(__f)), _M_alloc(__a)
  {
  }

  __strand_op(__strand_op&& __s)
    : _M_func(std::move(__s._M_func)), _M_alloc(std::move(__s._M_alloc))
  {
  }

  virtual void _Complete()
  {
    auto __op(_Adopt_small_block(_M_alloc, this));
    _Func __tmp(std::move(_M_func));
    __op.reset();
    std::move(__tmp)();
  }

  virtual void _Destroy()
  {
    _Adopt_small_block(_M_alloc, this);
  }

private:
  _Func _M_func;
  _Alloc _M_alloc;
};

template <class _Executor>
struct __strand_invoker
{
  executor_work<_Executor> _M_work;
  shared_ptr<__strand_impl> _M_impl;

  __strand_invoker(const _Executor& __e, const shared_ptr<__strand_impl>& __i)
    : _M_work(__e), _M_impl(__i)
  {
  }

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
        auto __executor(_M_invoker->_M_work.get_executor());
        __executor.defer(std::move(*_M_invoker), __small_block_allocator<void, __strand_impl>());
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

template <class _Executor> template <class _Func, class _Alloc>
void strand<_Executor>::dispatch(_Func&& __f, const _Alloc& __a)
{
  typedef typename decay<_Func>::type _DecayFunc;
  if (__call_stack<__strand_impl>::_Contains(_M_impl.get()))
  {
    _DecayFunc(forward<_Func>(__f))();
    return;
  }

  typedef typename decay<_Func>::type _DecayFunc;
  auto __op(_Allocate_small_block<__strand_op<_DecayFunc, _Alloc>>(__a, forward<_Func>(__f), __a));

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

  _M_executor.dispatch(__strand_invoker<_Executor>(_M_executor, _M_impl), __small_block_allocator<void, __strand_impl>());
}

template <class _Executor> template <class _Func, class _Alloc>
void strand<_Executor>::post(_Func&& __f, const _Alloc& __a)
{
  typedef typename decay<_Func>::type _DecayFunc;
  auto __op(_Allocate_small_block<__strand_op<_DecayFunc, _Alloc>>(__a, forward<_Func>(__f), __a));

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

  _M_executor.post(__strand_invoker<_Executor>{_M_executor, _M_impl}, __small_block_allocator<void, __strand_impl>());
}

template <class _Executor> template <class _Func, class _Alloc>
inline void strand<_Executor>::defer(_Func&& __f, const _Alloc& __a)
{
  typedef typename decay<_Func>::type _DecayFunc;
  auto __op(_Allocate_small_block<__strand_op<_DecayFunc, _Alloc>>(__a, forward<_Func>(__f), __a));

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

  _M_executor.defer(__strand_invoker<_Executor>{_M_executor, _M_impl}, __small_block_allocator<void, __strand_impl>());
}

template <class _Executor>
inline bool operator==(const strand<_Executor>& __a, const strand<_Executor>& __b) noexcept
{
  return __a._M_impl == __b._M_impl;
}

template <class _Executor>
inline bool operator!=(const strand<_Executor>& __a, const strand<_Executor>& __b) noexcept
{
  return !(__a == __b);
}

template <class _T> inline auto make_strand(_T&& __t)
{
  return strand<typename decay<_T>::type>(forward<_T>(__t));
}

} // inline namespace concurrency_v1
} // namespace experimental
} // namespace std

#endif
