//
// continuation.h
// ~~~~~~~~~~~~~~
// Polymorphic continuation wrapper.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_CONTINUATION_H
#define EXECUTORS_EXPERIMENTAL_BITS_CONTINUATION_H

#include <experimental/executor>
#include <experimental/bits/function_traits.h>

namespace std {
namespace experimental {
inline namespace concurrency_v1 {

class bad_continuation
  : public std::exception
{
public:
  bad_continuation() noexcept {}
  ~bad_continuation() noexcept {}

  virtual const char* what() const noexcept
  {
    return "bad continuation";
  }
};

class __continuation
{
public:
  virtual ~__continuation() {}
  virtual const type_info& _Target_type() = 0;
  virtual void* _Target() = 0;
  virtual const void* _Target() const = 0;
  virtual executor _Get_executor() const noexcept = 0;
};

template <class _Signature>
class __continuation_impl_base;

template <class _R, class... _Args>
class __continuation_impl_base<_R(_Args...)>
  : public __continuation
{
public:
  virtual void _Call(_Args... __args) = 0;
  virtual void _Call_with_continuation(_Args... __args,
    continuation<__make_signature_t<void, _R>>&& __c) = 0;
};

template <class _Continuation, class _Signature>
class __continuation_impl;

template <class _Continuation, class _R, class... _Args>
class __continuation_impl<_Continuation, _R(_Args...)>
  : public __continuation_impl_base<_R(_Args...)>
{
public:
  static_assert(__is_callable_with<_Continuation, void(_Args...)>::value,
    "continuation must be callable with the specified arguments");

  static_assert(__is_callable_with<typename continuation_of<_Continuation(_Args...)>::signature,
    __make_signature_t<void, _R>>::value,
    "continuation's continuation must accept specified return value");

  template <class _T> explicit __continuation_impl(_T&& __t)
    : _M_continuation(forward<_T>(__t)), _M_executor(associated_executor<_Continuation>::get(_M_continuation))
  {
  }

  virtual void _Call(_Args... __args)
  {
    std::move(_M_continuation)(forward<_Args>(__args)...);
  }

  virtual void _Call_with_continuation(_Args... __args,
    continuation<__make_signature_t<void, _R>>&& __c)
  {
    continuation_of<_Continuation(_Args...)>::chain(
      std::move(_M_continuation), std::move(__c))(
        forward<_Args>(__args)...);
  }

  virtual const type_info& _Target_type()
  {
    return typeid(_M_continuation);
  }

  virtual void* _Target()
  {
    return &_M_continuation;
  }

  virtual const void* _Target() const
  {
    return &_M_continuation;
  }

  virtual executor _Get_executor() const noexcept
  {
    return _M_executor;
  }

private:
  _Continuation _M_continuation;
  executor _M_executor;
};

template <class _R, class... _Args>
inline continuation<_R(_Args...)>::continuation() noexcept
{
}

template <class _R, class... _Args>
inline continuation<_R(_Args...)>::continuation(nullptr_t) noexcept
{
}

template <class _R, class... _Args>
inline continuation<_R(_Args...)>::continuation(continuation&& __c) noexcept
  : _M_impl(std::move(__c._M_impl))
{
}

template <class _R, class... _Args> template <class _Continuation>
inline continuation<_R(_Args...)>::continuation(_Continuation __c)
  : _M_impl(new __continuation_impl<_Continuation, _R(_Args...)>(std::move(__c)))
{
}

template <class _R, class... _Args> template <class _Alloc>
inline continuation<_R(_Args...)>::continuation(allocator_arg_t, const _Alloc&) noexcept
{
}

template <class _R, class... _Args> template <class _Alloc>
inline continuation<_R(_Args...)>::continuation(allocator_arg_t, const _Alloc&, nullptr_t) noexcept
{
}

template <class _R, class... _Args> template <class _Alloc>
inline continuation<_R(_Args...)>::continuation(allocator_arg_t, const _Alloc&, continuation&& __c) noexcept
  : _M_impl(std::move(__c._M_impl))
{
}

template <class _R, class... _Args> template <class _Continuation, class _Alloc>
inline continuation<_R(_Args...)>::continuation(allocator_arg_t, const _Alloc&, _Continuation __c)
  : _M_impl(new __continuation_impl<_Continuation, _R(_Args...)>(std::move(__c)))
{
}

template <class _R, class... _Args>
inline continuation<_R(_Args...)>& continuation<_R(_Args...)>::operator=(continuation&& __c)
{
  _M_impl = std::move(__c._M_impl);
  return *this;
}

template <class _R, class... _Args>
inline continuation<_R(_Args...)>& continuation<_R(_Args...)>::operator=(nullptr_t)
{
  _M_impl.reset();
  return *this;
}

template <class _R, class... _Args> template <class _Continuation>
continuation<_R(_Args...)>& continuation<_R(_Args...)>::operator=(_Continuation&& __c)
{
  _M_impl.reset(new __continuation_impl<_Continuation, _R(_Args...)>(std::move(__c)));
  return *this;
}

template <class _R, class... _Args>
inline continuation<_R(_Args...)>::~continuation()
{
}

template <class _R, class... _Args>
inline continuation<_R(_Args...)>::operator bool() const noexcept
{
  return static_cast<bool>(_M_impl);
}

template <class _R, class... _Args>
inline typename continuation<_R(_Args...)>::executor_type
continuation<_R(_Args...)>::get_executor() const noexcept
{
  return _M_impl ? _M_impl->_Get_executor() : executor_type();
}

template <class _R, class... _Args>
inline void continuation<_R(_Args...)>::operator()(_Args... __args)
{
  if (!_M_impl)
    throw bad_continuation();
  _M_impl->_Call(forward<_Args>(__args)...);
}

template <class _R, class... _Args>
inline const type_info& continuation<_R(_Args...)>::target_type() const noexcept
{
  return _M_impl->_Target_type();
}

template <class _R, class... _Args> template <class _Continuation>
inline _Continuation* continuation<_R(_Args...)>::target() noexcept
{
  return static_cast<_Continuation*>(_M_impl->_Target());
}

template <class _R, class... _Args> template <class _Continuation>
inline const _Continuation* continuation<_R(_Args...)>::target() const noexcept
{
  return static_cast<_Continuation*>(_M_impl->_Target());
}

template <class _R, class... _Args>
struct __continuation_chain
{
  continuation<_R(_Args...)> _M_head;
  continuation<__make_signature_t<void, _R>> _M_tail;

  void operator()(_Args... __args)
  {
    _M_head._M_impl->_Call_with_continuation(forward<_Args>(__args)..., std::move(_M_tail));
  }
};

template <class _R, class... _Args, class... _Args2>
struct continuation_of<continuation<_R(_Args...)>(_Args2...)>
{
  typedef __make_signature_t<void, _R> signature;

  template <class _C>
  static auto chain(continuation<_R(_Args...)>&& __f, _C&& __c)
  {
    return __continuation_chain<_R, _Args...>{std::move(__f), forward<_C>(__c)};
  }
};

template <class _R, class... _Args>
inline bool operator==(const continuation<_R(_Args...)>& __c, nullptr_t) noexcept
{
  return !static_cast<bool>(__c);
}

template <class _R, class... _Args>
inline bool operator==(nullptr_t, const continuation<_R(_Args...)>& __c) noexcept
{
  return !static_cast<bool>(__c);
}

template <class _R, class... _Args>
inline bool operator!=(const continuation<_R(_Args...)>& __c, nullptr_t) noexcept
{
  return static_cast<bool>(__c);
}

template <class _R, class... _Args>
inline bool operator!=(nullptr_t, const continuation<_R(_Args...)>& __c) noexcept
{
  return static_cast<bool>(__c);
}

inline continuation<>::continuation() noexcept
{
}

inline continuation<>::continuation(nullptr_t) noexcept
{
}

inline continuation<>::continuation(continuation&& __c) noexcept
  : _M_impl(std::move(__c._M_impl))
{
}

template <class _R, class... _Args>
inline continuation<>::continuation(continuation<_R(_Args...)>&& __c) noexcept
  : _M_impl(std::move(__c._M_impl))
{
}

template <class _Alloc>
inline continuation<>::continuation(allocator_arg_t, const _Alloc&) noexcept
{
}

template <class _Alloc>
inline continuation<>::continuation(allocator_arg_t, const _Alloc&, nullptr_t) noexcept
{
}

template <class _Alloc>
inline continuation<>::continuation(allocator_arg_t, const _Alloc&, continuation&& __c) noexcept
  : _M_impl(std::move(__c._M_impl))
{
}

template <class _R, class... _Args, class _Alloc>
inline continuation<>::continuation(allocator_arg_t, const _Alloc&, continuation<_R(_Args...)>&& __c) noexcept
  : _M_impl(std::move(__c._M_impl))
{
}

inline continuation<>& continuation<>::operator=(continuation&& __c)
{
  _M_impl = std::move(__c._M_impl);
  return *this;
}

inline continuation<>& continuation<>::operator=(nullptr_t)
{
  _M_impl.reset();
  return *this;
}

template <class _R, class... _Args>
continuation<>& continuation<>::operator=(continuation<_R(_Args...)>&& __c)
{
  _M_impl = std::move(__c._M_impl);
  return *this;
}

inline continuation<>::~continuation()
{
}

inline continuation<>::operator bool() const noexcept
{
  return static_cast<bool>(_M_impl);
}

inline continuation<>::executor_type continuation<>::get_executor() const noexcept
{
  return _M_impl ? _M_impl->_Get_executor() : executor_type();
}

inline const type_info& continuation<>::target_type() const noexcept
{
  return _M_impl->_Target_type();
}

template <class _Continuation>
inline _Continuation* continuation<>::target() noexcept
{
  return static_cast<_Continuation*>(_M_impl->_Target());
}

template <class _Continuation>
inline const _Continuation* continuation<>::target() const noexcept
{
  return static_cast<_Continuation*>(_M_impl->_Target());
}

inline bool operator==(const continuation<>& __c, nullptr_t) noexcept
{
  return !static_cast<bool>(__c);
}

inline bool operator==(nullptr_t, const continuation<>& __c) noexcept
{
  return !static_cast<bool>(__c);
}

inline bool operator!=(const continuation<>& __c, nullptr_t) noexcept
{
  return static_cast<bool>(__c);
}

inline bool operator!=(nullptr_t, const continuation<>& __c) noexcept
{
  return static_cast<bool>(__c);
}

template <class _Signature>
inline continuation<_Signature> static_continuation_cast(continuation<>&& __c)
{
  return continuation<_Signature>(
    static_cast<__continuation_impl_base<_Signature>*>(__c._M_impl.release()));
}

template <class _Signature>
inline continuation<_Signature> dynamic_continuation_cast(continuation<>&& __c)
{
  return dynamic_cast<__continuation_impl_base<_Signature>*>(__c._M_impl.get()) ?
    static_continuation_cast<_Signature>(std::move(__c)) : continuation<_Signature>();
}

template <class _Signature>
struct __continuation_handler
  : continuation<_Signature>
{
  __continuation_handler(continuation<>&& __c)
    : continuation<_Signature>(dynamic_continuation_cast<_Signature>(std::move(__c)))
  {
  }
};

template <class _Signature>
class async_result<__continuation_handler<_Signature>>
{
public:
  typedef continuation_result<_Signature> type;

  async_result(__continuation_handler<_Signature>&)
  {
  }

  async_result(const async_result&) = delete;
  async_result& operator=(const async_result&) = delete;

  type get()
  {
    return type();
  }
};

template <class _R, class... _Args>
struct handler_type<continuation<>, _R(_Args...)>
{
  typedef __continuation_handler<_R(_Args...)> type;
};

template <class _Func, class _Signature>
struct __continuation_launcher
{
  _Func _M_func;
  continuation<_Signature> _M_continuation;

  template <class _F> explicit __continuation_launcher(_F&& __f)
    : _M_func(forward<_F>(__f))
  {
  }

  template <class _F, class _C> __continuation_launcher(_F&& __f, _C&& __c)
    : _M_func(forward<_F>(__f)), _M_continuation(forward<_C>(__c))
  {
  }

  template <class... _Args> void operator()(_Args&&... __args)
  {
    std::move(_M_func)(forward<_Args>(__args)..., std::move(_M_continuation));
  }
};

template <class _Func, class _Signature, class... _Args>
struct continuation_of<__continuation_launcher<_Func, _Signature>(_Args...)>
{
  typedef _Signature signature;

  template <class _Continuation>
  static auto chain(__continuation_launcher<_Func, _Signature>&& __f, _Continuation&& __c)
  {
    return __continuation_launcher<_Func, _Signature>(
      std::move(__f._M_func), forward<_Continuation>(__c));
  }
};

template <class _T, class = void>
struct __has_continuation : false_type {};

template <class _T>
struct __has_continuation<_T,
  typename enable_if<is_convertible<__last_argument_t<__signature_t<_T>>,
    continuation<>>::value>::type> : true_type {};

template <class _Result, class _LastArgument>
struct __continuation_result_signature
{
  static_assert(!sizeof(_Result*),
    "A continuation<>-accepting function must return the result of the tail-call operation.");
};

template <class _Signature, class _LastArgument>
struct __continuation_result_signature<continuation_result<_Signature>, _LastArgument>
{
  typedef _Signature signature;
};

template <class _Signature>
struct __continuation_result_signature<void, continuation<_Signature>>
{
  typedef _Signature signature;
};

template <class _Signature>
struct __continuation_result_signature<continuation_result<_Signature>, continuation<_Signature>>
{
  typedef _Signature signature;
};

template <class _Signature1, class _Signature2>
struct __continuation_result_signature<continuation_result<_Signature1>, continuation<_Signature2>>
{
  static_assert(!sizeof(_Signature1*),
    "Mismatch between result signature and signature of last argument.");
};

template <class _Func, class _R, class... _Args>
struct handler_type<_Func, _R(_Args...),
  typename enable_if<__has_continuation<typename decay<_Func>::type>::value
    && !__is_executor_wrapper<typename decay<_Func>::type>::value>::type>
{
  typedef typename decay<_Func>::type _DecayFunc;
  typedef __result_t<__signature_t<_DecayFunc>> _Result;
  typedef __last_argument_t<__signature_t<_DecayFunc>> _LastArgument;
  typedef typename __continuation_result_signature<_Result, _LastArgument>::signature _Signature;
  typedef __continuation_launcher<_DecayFunc, _Signature> type;
};

} // inline namespace concurrency_v1
} // namespace experimental
} // namespace std

#endif
