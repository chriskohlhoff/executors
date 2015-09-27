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
inline namespace concurrency_v2 {

template <class _R, class... _Args, class _Signature>
class async_result<packaged_task<_R(_Args...)>, _Signature>
{
public:
  typedef packaged_task<_R(_Args...)> completion_handler_type;
  typedef future<_R> return_type;

  async_result(completion_handler_type& __h) : _M_future(__h.get_future()) {}
  async_result(const async_result&) = delete;
  async_result& operator=(const async_result&) = delete;

  return_type get() { return std::move(_M_future); }

private:
  return_type _M_future;
};

} // inline namespace concurrency_v2
} // namespace experimental
} // namespace std

#endif
