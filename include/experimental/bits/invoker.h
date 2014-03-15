//
// invoker.h
// ~~~~~~~~~
// Function objects to adapt other functions to executor requirements.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_INVOKER_H
#define EXECUTORS_EXPERIMENTAL_BITS_INVOKER_H

namespace std {
namespace experimental {

struct __empty_function_void0 { void operator()() {} };

template <class _Result, class _Consumer>
struct __invoke_with_result
{
  _Result _M_result;
  _Consumer _M_consumer;

  void operator()()
  {
    _M_consumer(std::move(_M_result));
  }
};

template <class _Producer, class _Consumer, class _Signature>
struct __invoker;

template <class _Producer, class _Consumer, class _Result, class... _Args>
struct __invoker<_Producer, _Consumer, _Result(_Args...)>
{
  typedef __make_signature_t<void, _Result> _ConsumerSignature;

  _Producer _M_producer;
  _Consumer _M_consumer;
  typename decltype(get_executor(declval<_Consumer>()))::work _M_consumer_work;

  void operator()(_Args... __args)
  {
    get_executor(_M_consumer_work).dispatch(
      __invoke_with_result<_Result, _Consumer>{
        _M_producer(forward<_Args>(__args)...), std::move(_M_consumer)});
  }
};

template <class _Producer, class _Consumer, class... _Args>
struct __invoker<_Producer, _Consumer, void(_Args...)>
{
  typedef __make_signature_t<void> _ConsumerSignature;

  _Producer _M_producer;
  _Consumer _M_consumer;
  typename decltype(get_executor(declval<_Consumer>()))::work _M_consumer_work;

  void operator()(_Args... __args)
  {
    _M_producer(forward<_Args>(__args)...);
    get_executor(_M_consumer_work).dispatch(std::move(_M_consumer));
  }
};

struct __invoker_producer_executor
{
  template <class _Producer, class _Consumer, class _Signature>
  static auto _Get(const __invoker<_Producer, _Consumer, _Signature>& __i)
  {
    return get_executor(__i._M_producer);
  }
};

struct __invoker_consumer_executor
{
  template <class _Producer, class _Consumer, class _Signature>
  static auto _Get(const __invoker<_Producer, _Consumer, _Signature>& __i)
  {
    return get_executor(__i._M_consumer);
  }
};

template <class _Producer, class _Consumer, class _Signature>
inline auto get_executor(const __invoker<_Producer, _Consumer, _Signature>& __i)
{
  typedef decltype(get_executor(__i._M_producer)) _ProducerExecutor;
  return conditional<is_same<_ProducerExecutor, system_executor>::value,
    __invoker_consumer_executor, __invoker_producer_executor>::type::_Get(__i);
}

} // namespace experimental
} // namespace std

#endif
