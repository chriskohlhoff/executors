//
// executor_binder.h
// ~~~~~~~~~~~~~~~~~
// Binds an executor to an object.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_EXECUTOR_WRAPPER_H
#define EXECUTORS_EXPERIMENTAL_BITS_EXECUTOR_WRAPPER_H

namespace std {
namespace experimental {
inline namespace concurrency_v2 {

template <class _T, class _Executor>
inline executor_binder<_T, _Executor>::executor_binder(_T __t, const _Executor& __ex)
  : executor_binder(0, __ex, std::move(__t))
{
}

template <class _T, class _Executor> template <class _U, class _OtherExecutor>
inline executor_binder<_T, _Executor>::executor_binder(const executor_binder<_U, _OtherExecutor>& __w)
  : executor_binder(0, __w._M_executor, __w._M_target)
{
}

template <class _T, class _Executor> template <class _U, class _OtherExecutor>
inline executor_binder<_T, _Executor>::executor_binder(executor_binder<_U, _OtherExecutor>&& __w)
  : executor_binder(0, __w._M_executor, std::move(__w._M_target))
{
}

template <class _T, class _Executor> template <class _U, class _OtherExecutor>
inline executor_binder<_T, _Executor>::executor_binder(executor_arg_t,
  const _Executor& __ex, const executor_binder<_U, _OtherExecutor>& __w)
    : executor_binder(0, __ex, __w._M_target)
{
}

template <class _T, class _Executor> template <class _U, class _OtherExecutor>
inline executor_binder<_T, _Executor>::executor_binder(executor_arg_t,
  const _Executor& __ex, executor_binder<_U, _OtherExecutor>&& __w)
    : executor_binder(0, __ex, std::move(__w._M_target))
{
}

template <class _T, class _Executor> template <class _E, class _U>
inline executor_binder<_T, _Executor>::executor_binder(int, _E&& __e, _U&& __u)
  : executor_binder(forward<_E>(__e), forward<_U>(__u), uses_executor<_T, _Executor>())
{
}

template <class _T, class _Executor> template <class _E, class _U>
inline executor_binder<_T, _Executor>::executor_binder(_E&& __e, _U&& __u, true_type)
  : __executor_binder_base_executor<_Executor>(forward<_E>(__e)),
    __executor_binder_base_target<_T>(executor_arg, this->_M_executor, forward<_U>(__u))
{
}

template <class _T, class _Executor> template <class _E, class _U>
inline executor_binder<_T, _Executor>::executor_binder(_E&& __e, _U&& __u, false_type)
  : __executor_binder_base_executor<_Executor>(forward<_E>(__e)),
    __executor_binder_base_target<_T>(forward<_U>(__u))
{
}

template <class _T, class _Executor>
inline executor_binder<_T, _Executor>::~executor_binder()
{
}

template <class _T, class _Executor>
inline _T& executor_binder<_T, _Executor>::get() noexcept
{
  return this->_M_target;
}

template <class _T, class _Executor>
inline const _T& executor_binder<_T, _Executor>::get() const noexcept
{
  return this->_M_target;
}

template <class _T, class _Executor>
inline typename executor_binder<_T, _Executor>::executor_type
executor_binder<_T, _Executor>::get_executor() const noexcept
{
  return this->_M_executor;
}

template <class _T, class _Executor> template <class... _Args>
inline typename result_of<_T&(_Args&&...)>::type
executor_binder<_T, _Executor>::operator()(_Args&&... __args)
{
  return this->_M_target(forward<_Args>(__args)...);
}

template <class _T, class _Executor> template <class... _Args>
inline typename result_of<const _T&(_Args&&...)>::type
executor_binder<_T, _Executor>::operator()(_Args&&... __args) const
{
  return this->_M_target(forward<_Args>(__args)...);
}

template <class _T, class _Executor, class _Signature>
class async_result<executor_binder<_T, _Executor>, _Signature>
{
public:
  typedef executor_binder<
    typename async_result<_T, _Signature>::completion_handler_type,
      _Executor> completion_handler_type;

  typedef typename async_result<_T, _Signature>::return_type return_type;

  explicit async_result(completion_handler_type& __h) : _M_target(__h.get()) {}
  return_type get() { return _M_target.get(); }

private:
  async_result<_T, _Signature> _M_target;
};

template <class _T, class _Executor, class _Alloc>
struct associated_allocator<executor_binder<_T, _Executor>, _Alloc>
{
  typedef typename associated_allocator<_T, _Alloc>::type type;

  static type get(const executor_binder<_T, _Executor>& __w, const _Alloc& __a = _Alloc()) noexcept
  {
    return associated_allocator<_T, _Alloc>::get(__w.get(), __a);
  }
};

template <class _T, class _Executor, class _Executor1>
struct associated_executor<executor_binder<_T, _Executor>, _Executor1>
{
  typedef _Executor type;

  static type get(const executor_binder<_T, _Executor>& __w, const _Executor1& = _Executor1()) noexcept
  {
    return __w.get_executor();
  }
};

} // inline namespace concurrency_v2
} // namespace experimental
} // namespace std

#endif
