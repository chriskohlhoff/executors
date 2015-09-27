//
// work_dispatcher.h
// ~~~~~~~~~~~~~~~~~
// Function object used to implement post, dispatch, etc.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_WORK_DISPATCHER_H
#define EXECUTORS_EXPERIMENTAL_BITS_WORK_DISPATCHER_H

namespace std {
namespace experimental {
inline namespace concurrency_v2 {

template <class _Handler>
class __work_dispatcher
{
public:
  explicit __work_dispatcher(_Handler& __h)
    : _M_work((get_associated_executor)(__h)),
      _M_handler(std::move(__h))
  {
  }

  void operator()()
  {
    auto __ex(_M_work.get_executor());
    auto __alloc((get_associated_allocator)(_M_handler));
    __ex.dispatch(std::move(_M_handler), __alloc);
    _M_work.reset();
  }

private:
  executor_work<typename associated_executor<_Handler>::type> _M_work;
  _Handler _M_handler;
};

} // inline namespace concurrency_v2
} // namespace experimental
} // namespace std

#endif
