//
// executor_wrapper.h
// ~~~~~~~~~~~~~~~~~~
// Wraps a function object so that it is associated with an executor.
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
inline namespace concurrency_v1 {

template <class _T, class _Executor>
inline executor_wrapper<_T, _Executor>::executor_wrapper(_T __t, const _Executor& __ex)
  : executor_wrapper(0, __ex, std::move(__t))
{
}

template <class _T, class _Executor> template <class _U, class _OtherExecutor>
inline executor_wrapper<_T, _Executor>::executor_wrapper(const executor_wrapper<_U, _OtherExecutor>& __w)
  : executor_wrapper(0, __w._M_executor, __w._M_wrapped)
{
}

template <class _T, class _Executor> template <class _U, class _OtherExecutor>
inline executor_wrapper<_T, _Executor>::executor_wrapper(executor_wrapper<_U, _OtherExecutor>&& __w)
  : executor_wrapper(0, __w._M_executor, std::move(__w._M_wrapped))
{
}

template <class _T, class _Executor> template <class _U, class _OtherExecutor>
inline executor_wrapper<_T, _Executor>::executor_wrapper(executor_arg_t,
  const _Executor& __ex, const executor_wrapper<_U, _OtherExecutor>& __w)
    : executor_wrapper(0, __ex, __w._M_wrapped)
{
}

template <class _T, class _Executor> template <class _U, class _OtherExecutor>
inline executor_wrapper<_T, _Executor>::executor_wrapper(executor_arg_t,
  const _Executor& __ex, executor_wrapper<_U, _OtherExecutor>&& __w)
    : executor_wrapper(0, __ex, std::move(__w._M_wrapped))
{
}

template <class _T, class _Executor> template <class _E, class _U>
inline executor_wrapper<_T, _Executor>::executor_wrapper(int, _E&& __e, _U&& __u)
  : executor_wrapper(forward<_E>(__e), forward<_U>(__u), uses_executor<_T, _Executor>())
{
}

template <class _T, class _Executor> template <class _E, class _U>
inline executor_wrapper<_T, _Executor>::executor_wrapper(_E&& __e, _U&& __u, true_type)
  : __executor_wrapper_base_executor<_Executor>(forward<_E>(__e)),
    __executor_wrapper_base_wrapped<_T>(executor_arg, this->_M_executor, forward<_U>(__u))
{
}

template <class _T, class _Executor> template <class _E, class _U>
inline executor_wrapper<_T, _Executor>::executor_wrapper(_E&& __e, _U&& __u, false_type)
  : __executor_wrapper_base_executor<_Executor>(forward<_E>(__e)),
    __executor_wrapper_base_wrapped<_T>(forward<_U>(__u))
{
}

template <class _T, class _Executor>
inline executor_wrapper<_T, _Executor>::~executor_wrapper()
{
}

template <class _T, class _Executor>
inline _T& executor_wrapper<_T, _Executor>::unwrap() noexcept
{
  return this->_M_wrapped;
}

template <class _T, class _Executor>
inline const _T& executor_wrapper<_T, _Executor>::unwrap() const noexcept
{
  return this->_M_wrapped;
}

template <class _T, class _Executor>
inline typename executor_wrapper<_T, _Executor>::executor_type
executor_wrapper<_T, _Executor>::get_executor() const noexcept
{
  return this->_M_executor;
}

template <class _T, class _Executor> template <class... _Args>
inline typename result_of<_T&(_Args&&...)>::type
executor_wrapper<_T, _Executor>::operator()(_Args&&... __args) &
{
  return this->_M_wrapped(forward<_Args>(__args)...);
}

template <class _T, class _Executor> template <class... _Args>
inline typename result_of<const _T&(_Args&&...)>::type
executor_wrapper<_T, _Executor>::operator()(_Args&&... __args) const &
{
  return this->_M_wrapped(forward<_Args>(__args)...);
}

template <class _T, class _Executor> template <class... _Args>
inline typename result_of<_T&&(_Args&&...)>::type
executor_wrapper<_T, _Executor>::operator()(_Args&&... __args) &&
{
  return std::move(this->_M_wrapped)(forward<_Args>(__args)...);
}

template <class _T, class _Executor> template <class... _Args>
inline typename result_of<const _T&&(_Args&&...)>::type
executor_wrapper<_T, _Executor>::operator()(_Args&&... __args) const &&
{
  return std::move(this->_M_wrapped)(forward<_Args>(__args)...);
}

template <class _T, class _Executor, class _Signature>
struct handler_type<executor_wrapper<_T, _Executor>, _Signature>
{
  typedef executor_wrapper<handler_type_t<_T, _Signature>, _Executor> type;
};

template <class _T, class _Executor>
class async_result<executor_wrapper<_T, _Executor>>
{
public:
  typedef typename async_result<_T>::type type;
  explicit async_result(executor_wrapper<_T, _Executor>& __w) : _M_wrapped(__w.unwrap()) {}
  type get() { return _M_wrapped.get(); }

private:
  async_result<_T> _M_wrapped;
};

template <class _T, class _Executor, class _Alloc>
struct associated_allocator<executor_wrapper<_T, _Executor>, _Alloc>
{
  typedef typename associated_allocator<_T, _Alloc>::type type;

  static type get(const executor_wrapper<_T, _Executor>& __w, const _Alloc& __a = _Alloc()) noexcept
  {
    return associated_allocator<_T, _Alloc>::get(__w.unwrap(), __a);
  }
};

template <class _T, class _Executor, class _Executor1>
struct associated_executor<executor_wrapper<_T, _Executor>, _Executor1>
{
  typedef _Executor type;

  static type get(const executor_wrapper<_T, _Executor>& __w, const _Executor1& = _Executor1()) noexcept
  {
    return __w.get_executor();
  }
};

template <class _T>
struct __is_executor_wrapper : false_type {};

template <class _T, class _Executor>
struct __is_executor_wrapper<executor_wrapper<_T, _Executor>> : true_type {};

template <class _T, class _Executor, class... _Args>
struct continuation_of<executor_wrapper<_T, _Executor>(_Args...)>
{
  typedef continuation_of<
    typename executor_wrapper<_T, _Executor>::wrapped_type(_Args...)> _Wrapped_continuation_of;

  typedef typename _Wrapped_continuation_of::signature signature;

  template <class _C>
  static auto chain(executor_wrapper<_T, _Executor>&& __f, _C&& __c)
  {
    return (wrap)(__f.get_executor(),
      _Wrapped_continuation_of::chain(std::move(__f.unwrap()),
        forward<_C>(__c)));
  }
};

} // inline namespace concurrency_v1
} // namespace experimental
} // namespace std

#endif
