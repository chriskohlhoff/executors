//
// executor.h
// ~~~~~~~~~~
// Polymorphic executor wrapper implementation.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_EXECUTOR_H
#define EXECUTORS_EXPERIMENTAL_BITS_EXECUTOR_H

#include <atomic>
#include <memory>
#include <experimental/bits/small_block_recycler.h>

namespace std {
namespace experimental {
inline namespace concurrency_v1 {

class __executor_impl_base;

class __function_base
{
public:
  virtual ~__function_base() {}
  virtual void _Invoke() = 0;
  virtual void _Destroy() = 0;
};

template <class _Func, class _Alloc>
class __function
  : public __function_base
{
public:
  template <class _F> __function(_F&& __f, const _Alloc& __a)
    : _M_func(forward<_F>(__f)), _M_alloc(__a)
  {
  }

  virtual void _Invoke()
  {
    auto __op(_Adopt_small_block(_M_alloc, this));
    _Func __f(std::move(_M_func));
    __op.reset();
    std::move(__f)();
  }

  virtual void _Destroy()
  {
    _Adopt_small_block(_M_alloc, this);
  }

private:
  _Func _M_func;
  _Alloc _M_alloc;
};

class __function_ptr
{
public:
  template <class _F, class _Alloc> __function_ptr(_F __f, const _Alloc& __a)
    : _M_func(_Allocate_small_block<__function<_F, _Alloc>>(__a, std::move(__f), __a).release())
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
    __function_base* __f = _M_func;
    _M_func = nullptr;
    __f->_Invoke();
  }

private:
  __function_base* _M_func;
};

class __executor_impl_base
{
public:
  virtual __executor_impl_base* _Clone() const noexcept = 0;
  virtual void _Destroy() noexcept = 0;
  virtual execution_context& _Context() = 0;
  virtual void _Work_started() noexcept = 0;
  virtual void _Work_finished() noexcept = 0;
  virtual void _Dispatch(__function_ptr&& __f) = 0;
  virtual void _Post(__function_ptr&& __f) = 0;
  virtual void _Defer(__function_ptr&& __f) = 0;
  virtual const type_info& _Target_type() const = 0;
  virtual void* _Target() = 0;
  virtual const void* _Target() const = 0;
  virtual bool _Equals(const __executor_impl_base* __e) const noexcept = 0;

protected:
  virtual ~__executor_impl_base() {}
};

template <class _Executor>
class __executor_impl
  : public __executor_impl_base
{
public:
  static __executor_impl_base* _Create(const _Executor& __e)
  {
    return new __executor_impl(__e);
  }

  virtual __executor_impl_base* _Clone() const noexcept
  {
    __executor_impl* __e = const_cast<__executor_impl*>(this);
    ++__e->_M_ref_count;
    return __e;
  }

  virtual void _Destroy() noexcept
  {
    if (--_M_ref_count == 0)
      delete this;
  }

  virtual execution_context& _Context()
  {
    return _M_executor.context();
  }

  virtual void _Work_started() noexcept
  {
    _M_executor.on_work_started();
  }

  virtual void _Work_finished() noexcept
  {
    _M_executor.on_work_finished();
  }

  virtual void _Dispatch(__function_ptr&& __f)
  {
    _M_executor.dispatch(std::move(__f), __small_block_allocator<void>());
  }

  virtual void _Post(__function_ptr&& __f)
  {
    _M_executor.post(std::move(__f), __small_block_allocator<void>());
  }

  virtual void _Defer(__function_ptr&& __f)
  {
    _M_executor.defer(std::move(__f), __small_block_allocator<void>());
  }

  virtual const type_info& _Target_type() const
  {
    return typeid(_M_executor);
  }

  virtual void* _Target()
  {
    return &_M_executor;
  }

  virtual const void* _Target() const
  {
    return &_M_executor;
  }

  virtual bool _Equals(const __executor_impl_base* __e) const noexcept
  {
    if (this == __e)
      return true;
    if (_Target_type() != __e->_Target_type())
      return false;
    return _M_executor == *static_cast<const _Executor*>(__e->_Target());
  }

private:
  explicit __executor_impl(const _Executor& __e) : _M_executor(__e), _M_ref_count(1) {}
  ~__executor_impl() {}
  _Executor _M_executor;
  atomic<size_t> _M_ref_count;
};

template <>
class __executor_impl<system_executor>
  : public __executor_impl_base
{
public:
  static __executor_impl_base* _Create()
  {
    static __executor_impl* __e = new __executor_impl;
    return __e;
  }

  static __executor_impl_base* _Create(const system_executor&)
  {
    return _Create();
  }

  virtual __executor_impl_base* _Clone() const noexcept
  {
    return const_cast<__executor_impl*>(this);
  }

  virtual void _Destroy() noexcept
  {
  }

  virtual execution_context& _Context()
  {
    return _M_executor.context();
  }

  virtual void _Work_started() noexcept
  {
    _M_executor.on_work_started();
  }

  virtual void _Work_finished() noexcept
  {
    _M_executor.on_work_started();
  }

  virtual void _Dispatch(__function_ptr&& __f)
  {
    _M_executor.dispatch(std::move(__f), __small_block_allocator<void>());
  }

  virtual void _Post(__function_ptr&& __f)
  {
    _M_executor.post(std::move(__f), __small_block_allocator<void>());
  }

  virtual void _Defer(__function_ptr&& __f)
  {
    _M_executor.defer(std::move(__f), __small_block_allocator<void>());
  }

  virtual const type_info& _Target_type() const
  {
    return typeid(system_executor);
  }

  virtual void* _Target()
  {
    return &_M_executor;
  }

  virtual const void* _Target() const
  {
    return &_M_executor;
  }

  virtual bool _Equals(const __executor_impl_base* __e) const noexcept
  {
    return __e == _Create();
  }

protected:
  __executor_impl() {}
  ~__executor_impl() {}
  system_executor _M_executor;
};

class bad_executor
  : public std::exception
{
public:
  bad_executor() noexcept {}
  ~bad_executor() noexcept {}

  virtual const char* what() const noexcept
  {
    return "bad executor";
  }
};

class __bad_executor_impl
  : public __executor_impl_base
{
public:
  static __executor_impl_base* _Create() noexcept
  {
    static __bad_executor_impl* __e = new __bad_executor_impl;
    return __e;
  }

  virtual __executor_impl_base* _Clone() const noexcept
  {
    return const_cast<__bad_executor_impl*>(this);
  }

  virtual void _Destroy() noexcept
  {
  }

  virtual execution_context& _Context()
  {
    return system_executor().context();
  }

  virtual void _Work_started() noexcept
  {
  }

  virtual void _Work_finished() noexcept
  {
  }

  virtual void _Dispatch(__function_ptr&&)
  {
    throw bad_executor();
  }

  virtual void _Post(__function_ptr&&)
  {
    throw bad_executor();
  }

  virtual void _Defer(__function_ptr&&)
  {
    throw bad_executor();
  }

  virtual const type_info& _Target_type() const
  {
    return typeid(void);
  }

  virtual void* _Target()
  {
    return nullptr;
  }

  virtual const void* _Target() const
  {
    return nullptr;
  }

  virtual bool _Equals(const __executor_impl_base*) const noexcept
  {
    return false;
  }

private:
  __bad_executor_impl() {}
  ~__bad_executor_impl() {}
};

inline executor::executor() noexcept
  : _M_impl(__bad_executor_impl::_Create())
{
}

inline executor::executor(nullptr_t) noexcept
  : _M_impl(__bad_executor_impl::_Create())
{
}

inline executor::executor(const executor& __e) noexcept
  : _M_impl(__e._M_impl->_Clone())
{
}

inline executor::executor(executor&& __e) noexcept
  : _M_impl(__e._M_impl)
{
  __e._M_impl = __bad_executor_impl::_Create();
}

template <class _Executor>
inline executor::executor(_Executor __e)
  : _M_impl(__executor_impl<_Executor>::_Create(std::move(__e)))
{
}

template <class _Executor, class _Alloc>
inline executor::executor(allocator_arg_t, const _Alloc&, _Executor __e)
  : _M_impl(__executor_impl<_Executor>::_Create(std::move(__e)))
{
}

inline executor::~executor()
{
  _M_impl->_Destroy();
}

inline executor& executor::operator=(const executor& __e) noexcept
{
  __executor_impl_base* __tmp = _M_impl;
  _M_impl = __e._M_impl->_Clone();
  __tmp->_Destroy();
  return *this;
}

inline executor& executor::operator=(executor&& __e) noexcept
{
  __executor_impl_base* __tmp = _M_impl;
  _M_impl = __e._M_impl;
  __tmp->_Destroy();
  __e._M_impl = __bad_executor_impl::_Create();
  return *this;
}

inline executor& executor::operator=(nullptr_t) noexcept
{
  _M_impl->_Destroy();
  _M_impl = __bad_executor_impl::_Create();
  return *this;
}

template <class _Executor>
inline executor& executor::operator=(_Executor __e)
{
  __executor_impl_base* __tmp = _M_impl;
  _M_impl = __executor_impl<typename decay<_Executor>::type>::_Create(forward<_Executor>(__e));
  __tmp->_Destroy();
  return *this;
}

inline execution_context& executor::context() noexcept
{
  return _M_impl->_Context();
}

inline void executor::on_work_started() noexcept
{
  _M_impl->_Work_started();
}

inline void executor::on_work_finished() noexcept
{
  _M_impl->_Work_finished();
}

template <class _Func, class _Alloc>
void executor::dispatch(_Func&& __f, const _Alloc& __a)
{
  if (static_cast<void*>(_M_impl) == static_cast<void*>(__executor_impl<system_executor>::_Create()))
    system_executor().dispatch(forward<_Func>(__f), __a);
  else
    _M_impl->_Dispatch(__function_ptr(forward<_Func>(__f), __a));
}

template <class _Func, class _Alloc>
inline void executor::post(_Func&& __f, const _Alloc& __a)
{
  _M_impl->_Post(__function_ptr(forward<_Func>(__f), __a));
}

template <class _Func, class _Alloc>
inline void executor::defer(_Func&& __f, const _Alloc& __a)
{
  _M_impl->_Defer(__function_ptr(forward<_Func>(__f), __a));
}

inline executor::operator bool() const noexcept
{
  return _M_impl != __bad_executor_impl::_Create();
}

inline const type_info& executor::target_type() const noexcept
{
  return _M_impl->_Target_type();
}

template <class _Executor>
inline _Executor* executor::target() noexcept
{
  return static_cast<_Executor*>(_M_impl->_Target());
}

template <class _Executor>
inline const _Executor* executor::target() const noexcept
{
  return static_cast<_Executor*>(_M_impl->_Target());
}

inline bool operator==(const executor& __a, const executor& __b) noexcept
{
  return __a._M_impl->_Equals(__b._M_impl);
}

inline bool operator==(const executor& __e, nullptr_t) noexcept
{
  return !static_cast<bool>(__e);
}

inline bool operator==(nullptr_t, const executor& __e) noexcept
{
  return !static_cast<bool>(__e);
}

inline bool operator!=(const executor& __a, const executor& __b) noexcept
{
  return !(__a == __b);
}

inline bool operator!=(const executor& __e, nullptr_t) noexcept
{
  return static_cast<bool>(__e);
}

inline bool operator!=(nullptr_t, const executor& __e) noexcept
{
  return static_cast<bool>(__e);
}

} // inline namespace concurrency_v1
} // namespace experimental
} // namespace std

#endif
