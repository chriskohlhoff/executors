//
// executor_impl.h
// ~~~~~~~~~~~~~~~
// Polymorphic executor implementation.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_EXECUTOR_IMPL_H
#define EXECUTORS_EXPERIMENTAL_BITS_EXECUTOR_IMPL_H

#include <exception>
#include <functional>

namespace std {
namespace experimental {

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
    return new __work_impl<typename _Executor::work>(_M_executor.make_work());
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

template <class _Executor>
class __executor_impl<reference_wrapper<_Executor>>
  : public __executor_impl_base
{
public:
  static __executor_impl_base* _Create(_Executor& __e)
  {
    return new __executor_impl(&__e);
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
    _M_executor->post(std::move(__f));
  }

  virtual void _Dispatch(function<void()>&& __f)
  {
    _M_executor->dispatch(std::move(__f));
  }

  virtual __work_impl_base* _Make_work()
  {
    return new __work_impl<typename _Executor::work>(_M_executor->make_work());
  }

  virtual const type_info& _Target_type()
  {
    return typeid(*_M_executor);
  }

  virtual void* _Target()
  {
    return _M_executor;
  }

  virtual const void* _Target() const
  {
    return _M_executor;
  }

private:
  explicit __executor_impl(_Executor* __e) : _M_executor(__e) {}
  ~__executor_impl();
  _Executor* _M_executor;
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

} // namespace experimental
} // namespace std

#endif
