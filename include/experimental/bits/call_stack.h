//
// call_stack.h
// ~~~~~~~~~~~~
// Helper to determine whether the caller is inside a given execution context.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_CALL_STACK_H
#define EXECUTORS_EXPERIMENTAL_BITS_CALL_STACK_H

namespace std {
namespace experimental {
inline namespace concurrency_v1 {

template <class _Key, class _Value = unsigned char>
class __call_stack
{
public:
  class __context
  {
  public:
    __context(const __context&) = delete;
    __context& operator=(const __context&) = delete;

    explicit __context(_Key* __k)
      : _M_key(__k), _M_next(__call_stack<_Key, _Value>::_S_top)
    {
      _M_value = reinterpret_cast<unsigned char*>(this);
      __call_stack<_Key, _Value>::_S_top = this;
    }

    __context(_Key* k, _Value& v)
      : _M_key(k), _M_value(&v), _M_next(__call_stack<_Key, _Value>::_S_top)
    {
      __call_stack<_Key, _Value>::_S_top = this;
    }

    ~__context()
    {
      __call_stack<_Key, _Value>::_S_top = _M_next;
    }

  private:
    friend class __call_stack<_Key, _Value>;
    _Key* _M_key;
    _Value* _M_value;
    __context* _M_next;
  };

  static _Value* _Contains(_Key* __k)
  {
    __context* __elem = _S_top;
    while (__elem)
    {
      if (__elem->_M_key == __k)
        return __elem->_M_value;
      __elem = __elem->_M_next;
    }
    return 0;
  }

  static _Value* _Top()
  {
    __context* __elem = _S_top;
    return __elem ? __elem->_M_value : 0;
  }

private:
  friend class __context;
#if defined(__APPLE__) && defined(__clang__)
  static __thread __context* _S_top;
#elif defined(_MSC_VER)
  static __declspec(thread) __context* _S_top;
#else
  static thread_local __context* _S_top;
#endif
};

#if defined(__APPLE__) && defined(__clang__)
template <class _Key, class _Value>
  __thread typename __call_stack<_Key, _Value>::__context*
    __call_stack<_Key, _Value>::_S_top;
#elif defined(_MSC_VER)
template <class _Key, class _Value>
  __declspec(thread) typename __call_stack<_Key, _Value>::__context*
    __call_stack<_Key, _Value>::_S_top;
#else
template <class _Key, class _Value>
  thread_local typename __call_stack<_Key, _Value>::__context*
    __call_stack<_Key, _Value>::_S_top;
#endif

} // inline namespace concurrency_v1
} // namespace experimental
} // namespace std

#endif
