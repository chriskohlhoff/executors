//
// unspecified_executor.h
// ~~~~~~~~~~~~~~~~~~~~~~
// Unspecified executor implementation.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_UNSPECIFIED_EXECUTOR_H
#define EXECUTORS_EXPERIMENTAL_BITS_UNSPECIFIED_EXECUTOR_H

namespace std {
namespace experimental {

template <class _Func>
inline void unspecified_executor::post(_Func&& __f)
{
  system_executor().post(forward<_Func>(__f));
}

template <class _Func>
void unspecified_executor::dispatch(_Func&& __f)
{
  system_executor().dispatch(forward<_Func>(__f));
}

inline unspecified_executor::work unspecified_executor::make_work()
{
  return work{};
}

template <class _Func>
inline auto unspecified_executor::wrap(_Func&& __f)
{
  return (wrap_with_executor)(forward<_Func>(__f), *this);
}

inline execution_context& unspecified_executor::context()
{
  return system_executor().context();
}

template <class _T>
inline unspecified_executor make_executor(const _T&,
  typename enable_if<__is_callable<_T>::value>::type*)
{
  return unspecified_executor();
}

inline unspecified_executor make_executor(const unspecified_executor&)
{
  return unspecified_executor();
}

inline unspecified_executor make_executor(unspecified_executor&&)
{
  return unspecified_executor();
}

inline unspecified_executor make_executor(const unspecified_executor::work&)
{
  return unspecified_executor();
}

inline unspecified_executor make_executor(unspecified_executor::work&&)
{
  return unspecified_executor();
}

} // namespace experimental
} // namespace std

#endif
