//
// packaged_task.h
// ~~~~~~~~~~~~~~~
// Support for packaged_task.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_PACKAGED_TASK_H
#define EXECUTORS_EXPERIMENTAL_BITS_PACKAGED_TASK_H

namespace std {
namespace experimental {
inline namespace concurrency_v1 {

template <class _Signature, class _Alloc>
class __packaged_task : public packaged_task<_Signature>
{
public:
  typedef _Alloc allocator_type;

  template <class _Func>
  explicit __packaged_task(package_t<_Func, _Alloc>&& __p)
    : packaged_task<_Signature>(allocator_arg, __p._M_allocator, std::move(__p._M_func)),
      _M_allocator(__p._M_allocator)
  {
  }

  allocator_type get_allocator() const noexcept
  {
    return _M_allocator;
  }

private:
  _Alloc _M_allocator;
};

template <class _Func, class _Alloc, class _R, class... _Args>
struct handler_type<package_t<_Func, _Alloc>, _R(_Args...)>
{
  typedef __packaged_task<typename result_of<_Func(_Args...)>::type(_Args...), _Alloc> type;
};

template <class _Signature, class _Alloc>
class async_result<__packaged_task<_Signature, _Alloc>>
  : public async_result<packaged_task<_Signature>>
{
public:
  async_result(__packaged_task<_Signature, _Alloc>& __h)
    : async_result<packaged_task<_Signature>>(__h) {}
};

template <class _F, class _Alloc>
inline package_t<typename decay<_F>::type, _Alloc> package(_F&& __f, const _Alloc& __a)
{
  return package_t<typename decay<_F>::type, _Alloc>(forward<_F>(__f), __a);
}

template <class _R, class... _Args>
class async_result<packaged_task<_R(_Args...)>>
{
public:
  typedef future<_R> type;

  async_result(packaged_task<_R(_Args...)>& __h)
    : _M_future(__h.get_future()) {}
  async_result(const async_result&) = delete;
  async_result& operator=(const async_result&) = delete;

  type get() { return std::move(_M_future); }

private:
  type _M_future;
};

} // inline namespace concurrency_v1
} // namespace experimental
} // namespace std

#endif
