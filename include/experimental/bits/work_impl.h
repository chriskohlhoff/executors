//
// work_impl.h
// ~~~~~~~~~~~
// Polymorphic executor work implementation.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_WORK_IMPL_H
#define EXECUTORS_EXPERIMENTAL_BITS_WORK_IMPL_H

namespace std {
namespace experimental {

class __work_impl_base
{
public:
  virtual __work_impl_base* _Clone() const = 0;
  virtual void _Destroy() = 0;

protected:
  virtual ~__work_impl_base() {}
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

private:
  explicit __work_impl(const _Work& __w) : _M_work(__w) {}
  ~__work_impl() {}
  _Work _M_work;
};

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

private:
  __work_impl() {}
  ~__work_impl() {}
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

private:
  __bad_work_impl() {}
  ~__bad_work_impl() {}
};

} // namespace experimental
} // namespace std

#endif
