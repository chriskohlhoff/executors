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

template <class _Executor>
inline executor::executor(reference_wrapper<_Executor> __e)
  : _M_impl(__executor_impl<reference_wrapper<_Executor>>::_Create(__e))
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

template <class _Executor, class _Alloc>
inline executor::executor(allocator_arg_t, const _Alloc&, reference_wrapper<_Executor> __e)
    : _M_impl(__executor_impl<reference_wrapper<_Executor>>::_Create(__e))
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

template <class _Executor>
inline executor& executor::operator=(reference_wrapper<_Executor> __e)
{
  __executor_impl_base* __tmp = _M_impl;
  _M_impl = __executor_impl<reference_wrapper<_Executor>>::_Create(__e);
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

} // namespace experimental
} // namespace std

#endif
