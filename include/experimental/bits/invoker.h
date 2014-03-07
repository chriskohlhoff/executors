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

template <class _Work, class _Result, class _Producer, class _Consumer>
struct __invoker
{
  _Work _M_work;
  _Producer _M_producer;
  _Consumer _M_consumer;
  void operator()()
  {
    get_executor(_M_work).dispatch(
      __invoker<_Work, _Result, void, _Consumer>{
        _M_work, _M_producer(), std::move(_M_consumer)});
  }
};

template <class _Work, class _Producer, class _Consumer>
struct __invoker<_Work, void, _Producer, _Consumer>
{
  _Work _M_work;
  _Producer _M_producer;
  _Consumer _M_consumer;
  void operator()()
  {
    _M_producer();
    get_executor(_M_work).dispatch(
      __invoker<_Work, void, void, _Consumer>{
        _M_work, std::move(_M_consumer)});
  }
};

template <class _Work, class _Result, class _Producer>
struct __invoker<_Work, _Result, _Producer, __empty_function_void0>
{
  _Work _M_work;
  _Producer _M_producer;
  __empty_function_void0 _M_consumer;
  void operator()() { _M_producer(); }
};

template <class _Work, class _Producer>
struct __invoker<_Work, void, _Producer, __empty_function_void0>
{
  _Work _M_work;
  _Producer _M_producer;
  __empty_function_void0 _M_consumer;
  void operator()() { _M_producer(); }
};

template <class _Work, class _Result, class _Consumer>
struct __invoker<_Work, _Result, void, _Consumer>
{
  _Work _M_work;
  _Result _M_result;
  _Consumer _M_consumer;
  void operator()() { _M_consumer(std::move(_M_result)); }
};

template <class _Work, class _Consumer>
struct __invoker<_Work, void, void, _Consumer>
{
  _Work _M_work;
  _Consumer _M_consumer;
  void operator()() { _M_consumer(); }
};

} // namespace experimental
} // namespace std

#endif
