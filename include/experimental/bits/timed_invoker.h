//
// timed_invoker.h
// ~~~~~~~~~~~~~~~
// Function object to invoke another function object after a delay.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_TIMED_INVOKER_H
#define EXECUTORS_EXPERIMENTAL_BITS_TIMED_INVOKER_H

#include <experimental/bits/invoker.h>

namespace std {
namespace experimental {
inline namespace concurrency_v1 {

template <class _Clock, class... _CompletionTokens>
class __timed_invoker
{
public:
  typedef __active_invoker<void(), _CompletionTokens...> _Tail;

  __timed_invoker(typename remove_reference<_CompletionTokens>::type&... __tokens)
    : _M_tail(__tokens...)
  {
  }

  template <class _Executor, class _Time>
  void _Start(const _Executor& __e, const _Time& __t)
  {
    _M_timer.reset(new basic_timer<_Clock>(_Executor(__e).context(), __t));
    basic_timer<_Clock>* __timer = _M_timer.get();
    __timer->wait((wrap)(__e, std::move(*this)));
  }

  void operator()(const error_code&)
  {
    std::move(_M_tail)();
  }

  _Tail& _Get_tail() noexcept
  {
    return _M_tail;
  }

private:
  unique_ptr<basic_timer<_Clock>> _M_timer;
  _Tail _M_tail;
};

} // inline namespace concurrency_v1
} // namespace experimental
} // namespace std

#endif
