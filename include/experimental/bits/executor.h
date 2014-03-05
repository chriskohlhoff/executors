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

inline executor::work::work(const work& __w)
  : _M_impl(__w._M_impl->_Clone())
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

template <class _Executor>
executor::executor(_Executor __e)
  : _M_impl(__executor_impl<_Executor>::_Create(std::move(__e)))
{
}

template <class _Executor, class _Alloc>
executor::executor(allocator_arg_t, const _Alloc&, _Executor __e)
  : _M_impl(__executor_impl<_Executor>::_Create(std::move(__e)))
{
}

template <class _Executor>
executor::executor(reference_wrapper<_Executor> __e)
  : _M_impl(__executor_impl<reference_wrapper<_Executor>>::_Create(__e))
{
}

template <class _Executor, class _Alloc>
executor::executor(allocator_arg_t, const _Alloc&, reference_wrapper<_Executor> __e)
  : _M_impl(__executor_impl<reference_wrapper<_Executor>>::_Create(__e))
{
}

inline executor::executor(const executor& __e)
  : _M_impl(__e._M_impl->_Clone())
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

template <class _Executor>
executor& executor::operator=(_Executor&& e)
{
  __executor_impl_base* __tmp = _M_impl;
  _M_impl = __executor_impl<_Executor>::_Create(e);
  __tmp->_Destroy();
  return *this;
}

template <class _Executor>
executor& executor::operator=(reference_wrapper<_Executor> e)
{
  __executor_impl_base* __tmp = _M_impl;
  _M_impl = __executor_impl<reference_wrapper<_Executor>>::_Create(e);
  __tmp->_Destroy();
  return *this;
}

template <class _Func>
void executor::post(_Func&& __f)
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

const type_info& executor::target_type() const noexcept
{
  return _M_impl->_Target_type();
}

template <class _Executor>
_Executor* executor::target() noexcept
{
  return static_cast<_Executor*>(_M_impl->_Target());
}

template <class _Executor>
const _Executor* executor::target() const noexcept
{
  return static_cast<_Executor*>(_M_impl->_Target());
}

} // namespace experimental
} // namespace std

#endif
