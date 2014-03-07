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

namespace std {
namespace experimental {

class __executor_impl_base;

class __work_impl_base
{
public:
  virtual __work_impl_base* _Clone() const = 0;
  virtual void _Destroy() = 0;
  virtual __executor_impl_base* _Get_executor() const = 0;

protected:
  virtual ~__work_impl_base() {}
};

class __executor_impl_base
{
public:
  virtual __executor_impl_base* _Clone() const = 0;
  virtual void _Destroy() = 0;
  virtual void _Post(function<void()>&& __f) = 0;
  virtual void _Dispatch(function<void()>&& __f) = 0;
  virtual __work_impl_base* _Make_work() = 0;
  virtual const type_info& _Target_type() = 0;
  virtual void* _Target() = 0;
  virtual const void* _Target() const = 0;

protected:
  virtual ~__executor_impl_base() {}
};

template <class _Work>
class __work_impl
  : public __work_impl_base
{
public:
  static __work_impl_base* _Create(const _Work& __w)
  {
    return new __work_impl(__w);
  }

  virtual __work_impl_base* _Clone() const
  {
    return new __work_impl(_M_work);
  }

  virtual void _Destroy()
  {
    delete this;
  }

  virtual __executor_impl_base* _Get_executor() const;

private:
  explicit __work_impl(const _Work& __w) : _M_work(__w) {}
  ~__work_impl() {}
  _Work _M_work;
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

  virtual __executor_impl_base* _Clone() const
  {
    return new __executor_impl(_M_executor);
  }

  virtual void _Destroy()
  {
    delete this;
  }

  virtual void _Post(function<void()>&& __f)
  {
    _M_executor.post(std::move(__f));
  }

  virtual void _Dispatch(function<void()>&& __f)
  {
    _M_executor.dispatch(std::move(__f));
  }

  virtual __work_impl_base* _Make_work()
  {
    return __work_impl<typename _Executor::work>::_Create(_M_executor.make_work());
  }

  virtual const type_info& _Target_type()
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

private:
  explicit __executor_impl(const _Executor& __e) : _M_executor(__e) {}
  ~__executor_impl() {}
  _Executor _M_executor;
};

template <class _Work>
inline __executor_impl_base* __work_impl<_Work>::_Get_executor() const
{
  typedef decltype(get_executor(_M_work)) _Executor;
  return __executor_impl<_Executor>::_Create(get_executor(_M_work));
}

template <>
class __work_impl<system_executor::work>
  : public __work_impl_base
{
public:
  static __work_impl_base* _Create()
  {
    static __work_impl __w;
    return &__w;
  }

  static __work_impl_base* _Create(system_executor::work)
  {
    return _Create();
  }

  virtual __work_impl_base* _Clone() const
  {
    return const_cast<__work_impl*>(this);
  }

  virtual void _Destroy()
  {
  }

  virtual __executor_impl_base* _Get_executor() const;

private:
  __work_impl() {}
  ~__work_impl() {}
};

template <>
class __executor_impl<system_executor>
  : public __executor_impl_base
{
public:
  static __executor_impl_base* _Create()
  {
    static __executor_impl __e;
    return &__e;
  }

  virtual __executor_impl_base* _Clone() const
  {
    return const_cast<__executor_impl*>(this);
  }

  virtual void _Destroy()
  {
  }

  virtual void _Post(function<void()>&& __f)
  {
    _M_executor.post(std::move(__f));
  }

  virtual void _Dispatch(function<void()>&& __f)
  {
    _M_executor.dispatch(std::move(__f));
  }

  virtual __work_impl_base* _Make_work()
  {
    return __work_impl<system_executor::work>::_Create();
  }

  virtual const type_info& _Target_type()
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

private:
  __executor_impl() {}
  ~__executor_impl() {}
  system_executor _M_executor;
};

template <>
inline __executor_impl_base* __work_impl<system_executor>::_Get_executor() const
{
  return __executor_impl<system_executor>::_Create();
}

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

class __bad_work_impl
  : public __work_impl_base
{
public:
  static __work_impl_base* _Create()
  {
    static __bad_work_impl __w;
    return &__w;
  }

  virtual __work_impl_base* _Clone() const
  {
    return const_cast<__bad_work_impl*>(this);
  }

  virtual void _Destroy()
  {
  }

  virtual __executor_impl_base* _Get_executor() const;

private:
  __bad_work_impl() {}
  ~__bad_work_impl() {}
};

class __bad_executor_impl
  : public __executor_impl_base
{
public:
  static __executor_impl_base* _Create()
  {
    static __bad_executor_impl __e;
    return &__e;
  }

  virtual __executor_impl_base* _Clone() const
  {
    return const_cast<__bad_executor_impl*>(this);
  }

  virtual void _Destroy()
  {
  }

  virtual void _Post(function<void()>&&)
  {
    throw bad_executor();
  }

  virtual void _Dispatch(function<void()>&&)
  {
    throw bad_executor();
  }

  virtual __work_impl_base* _Make_work()
  {
    return __bad_work_impl::_Create();
  }

  virtual const type_info& _Target_type()
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

private:
  __bad_executor_impl() {}
  ~__bad_executor_impl() {}
};

inline __executor_impl_base* __bad_work_impl::_Get_executor() const
{
  return __bad_executor_impl::_Create();
}

inline executor::executor() noexcept
  : _M_impl(__bad_executor_impl::_Create())
{
}

inline executor::executor(nullptr_t) noexcept
  : _M_impl(__bad_executor_impl::_Create())
{
}

inline executor::executor(const executor& __e)
  : _M_impl(__e._M_impl->_Clone())
{
}

inline executor::executor(executor&& __e)
  : _M_impl(__e._M_impl)
{
  __e._M_impl = __bad_executor_impl::_Create();
}

template <class _Executor>
inline executor::executor(_Executor __e)
  : _M_impl(__executor_impl<_Executor>::_Create(std::move(__e)))
{
}

template <class _Alloc>
inline executor::executor(allocator_arg_t, const _Alloc&) noexcept
  : _M_impl(__bad_executor_impl::_Create())
{
}

template <class _Alloc>
inline executor::executor(allocator_arg_t, const _Alloc&, nullptr_t) noexcept
  : _M_impl(__bad_executor_impl::_Create())
{
}

template <class _Alloc>
inline executor::executor(allocator_arg_t, const _Alloc&, const executor& __e)
  : _M_impl(__e._M_impl->_Clone())
{
}

template <class _Alloc>
inline executor::executor(allocator_arg_t, const _Alloc&, executor&& __e)
  : _M_impl(__e._M_impl)
{
  __e._M_impl = __bad_executor_impl::_Create();
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

inline executor& executor::operator=(const executor& __e)
{
  __executor_impl_base* __tmp = _M_impl;
  _M_impl = __e._M_impl->_Clone();
  __tmp->_Destroy();
  return *this;
}

inline executor& executor::operator=(executor&& __e)
{
  __executor_impl_base* __tmp = _M_impl;
  _M_impl = __e._M_impl;
  __tmp->_Destroy();
  __e._M_impl = __bad_executor_impl::_Create();
  return *this;
}

inline executor& executor::operator=(nullptr_t)
{
  _M_impl->_Destroy();
  _M_impl = __bad_executor_impl::_Create();
  return *this;
}

template <class _Executor>
inline executor& executor::operator=(_Executor&& __e)
{
  __executor_impl_base* __tmp = _M_impl;
  _M_impl = __executor_impl<typename decay<_Executor>::type>::_Create(forward<_Executor>(__e));
  __tmp->_Destroy();
  return *this;
}

template <class _Func>
inline void executor::post(_Func&& __f)
{
  _M_impl->_Post(function<void()>(forward<_Func>(__f)));
}

template <class _Func>
void executor::dispatch(_Func&& __f)
{
  if (static_cast<void*>(_M_impl) == static_cast<void*>(__executor_impl<system_executor>::_Create()))
    system_executor().dispatch(forward<_Func>(__f));
  else
    _M_impl->_Dispatch(function<void()>(forward<_Func>(__f)));
}

inline executor::work executor::make_work()
{
  return work(_M_impl->_Make_work());
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

inline executor get_executor(const executor& __e)
{
  return __e;
}

inline executor get_executor(executor&& __e)
{
  return std::move(__e);
}

inline bool operator==(const executor& __e, nullptr_t) noexcept
{
  return !static_cast<bool>(__e);
}

inline bool operator==(nullptr_t, const executor& __e) noexcept
{
  return !static_cast<bool>(__e);
}

inline bool operator!=(const executor& __e, nullptr_t) noexcept
{
  return static_cast<bool>(__e);
}

inline bool operator!=(nullptr_t, const executor& __e) noexcept
{
  return static_cast<bool>(__e);
}

inline executor::work::work() noexcept
  : _M_impl(__bad_work_impl::_Create())
{
}

inline executor::work::work(nullptr_t) noexcept
  : _M_impl(__bad_work_impl::_Create())
{
}

inline executor::work::work(const work& __w)
  : _M_impl(__w._M_impl->_Clone())
{
}

inline executor::work::work(work&& __w)
  : _M_impl(__w._M_impl)
{
  __w._M_impl = __bad_work_impl::_Create();
}

template <class _Work>
inline executor::work::work(_Work __w)
  : _M_impl(__work_impl<_Work>::_Create(__w))
{
}

template <class _Alloc>
inline executor::work::work(allocator_arg_t, const _Alloc&) noexcept
  : _M_impl(__bad_work_impl::_Create())
{
}

template <class _Alloc>
inline executor::work::work(allocator_arg_t, const _Alloc&, nullptr_t) noexcept
  : _M_impl(__bad_work_impl::_Create())
{
}

template <class _Alloc>
inline executor::work::work(allocator_arg_t, const _Alloc&, const work& __w)
    : _M_impl(__w._M_impl->_Clone())
{
}

template <class _Alloc>
inline executor::work::work(allocator_arg_t, const _Alloc&, work&& __w)
  : _M_impl(__w._M_impl)
{
  __w._M_impl = __bad_work_impl::_Create();
}

template <class _Work, class _Alloc>
inline executor::work::work(allocator_arg_t, const _Alloc&, _Work __w)
  : _M_impl(__work_impl<_Work>::_Create(__w))
{
}

inline executor::work::~work()
{
  _M_impl->_Destroy();
}

inline executor::work& executor::work::operator=(const work& __w)
{
  __work_impl_base* __tmp = _M_impl;
  _M_impl = __w._M_impl->_Clone();
  __tmp->_Destroy();
  return *this;
}

inline executor::work& executor::work::operator=(work&& __w)
{
  __work_impl_base* __tmp = _M_impl;
  _M_impl = __w._M_impl->_Clone();
  __tmp->_Destroy();
  __w._M_impl = __bad_work_impl::_Create();
  return *this;
}

inline executor::work& executor::work::operator=(nullptr_t)
{
  _M_impl->_Destroy();
  _M_impl = __bad_work_impl::_Create();
  return *this;
}

template <class _Work>
inline executor::work& executor::work::operator=(_Work&& __w)
{
  __work_impl_base* __tmp = _M_impl;
  _M_impl = __work_impl<typename decay<_Work>::type>::_Create(forward<_Work>(__w));
  __tmp->_Destroy();
  return *this;
}

inline executor::work::operator bool() const noexcept
{
  return _M_impl != __bad_work_impl::_Create();
}

inline executor get_executor(const executor::work& __w)
{
  return executor(__w._M_impl->_Get_executor());
}

inline executor get_executor(executor::work&& __w)
{
  return executor(__w._M_impl->_Get_executor());
}

inline bool operator==(const executor::work& __w, nullptr_t) noexcept
{
  return !static_cast<bool>(__w);
}

inline bool operator==(nullptr_t, const executor::work& __w) noexcept
{
  return !static_cast<bool>(__w);
}

inline bool operator!=(const executor::work& __w, nullptr_t) noexcept
{
  return static_cast<bool>(__w);
}

inline bool operator!=(nullptr_t, const executor::work& __w) noexcept
{
  return static_cast<bool>(__w);
}

} // namespace experimental
} // namespace std

#endif
