//
// function_traits.h
// ~~~~~~~~~~~~~~~~~
// Various traits for examining the attributes of function type.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_FUNCTION_TRAITS_H
#define EXECUTORS_EXPERIMENTAL_BITS_FUNCTION_TRAITS_H

#include <type_traits>
#include <utility>

namespace std {
namespace experimental {
inline namespace concurrency_v1 {

// __is_callable: Used to determine if a type is a function or function object.

struct __is_callable_base { void operator()(); };

template <class _T>
struct __is_callable_derived : _T, __is_callable_base {};

template <class _T, _T>
struct __is_callable_check { typedef void _Type; };

template <class _T, class = void>
struct __is_callable_class : true_type {};

template <class _T>
struct __is_callable_class<_T,
  typename __is_callable_check<void (__is_callable_base::*)(),
    &__is_callable_derived<_T>::operator()>::_Type> : false_type {};

template <class _T>
struct __is_callable_function :
  is_function<typename remove_pointer<typename remove_reference<_T>::type>::type> {};

template <class _T>
struct __is_callable :
  conditional<is_class<_T>::value,
    __is_callable_class<_T>, __is_callable_function<_T>>::type {};

// __is_callable_with: Determines whether a function object is callable with the
// specified signature.

template <class _T, class _Signature, class = void>
struct __is_callable_with : false_type {};

template <class _T, class _R, class... _Args>
struct __is_callable_with<_T, _R(_Args...), typename enable_if<
  is_convertible<typename result_of<_T&&(_Args...)>::type, _R>::value
    || is_same<_R, void>::value>::type> : true_type {};

// __signature: Makes a best effort at determining the signature of a function
// or function object. Works for functions, nullary function objects, and
// function objects that have a single (i.e. non-overloaded, non-templated)
// function call operator.

template <class _T>
struct __signature_check { typedef void type; };

template <class _T>
struct __signature_base { typedef _T type; };

template <class _T>
struct __signature_function {};

template <class _R, class... _Args>
struct __signature_function<_R(_Args...)> : __signature_base<_R(_Args...)> {};

template <class _R, class... _Args>
struct __signature_function<_R(*)(_Args...)> : __signature_base<_R(_Args...)> {};

template <class _R, class... _Args>
struct __signature_function<_R(&)(_Args...)> : __signature_base<_R(_Args...)> {};

template <class _R, class _C, class... _Args>
struct __signature_function<_R(_C::*)(_Args...)> : __signature_base<_R(_Args...)> {};

template <class _R, class _C, class... _Args>
struct __signature_function<_R(_C::*)(_Args...) &&> : __signature_base<_R(_Args...)> {};

template <class _R, class _C, class... _Args>
struct __signature_function<_R(_C::*)(_Args...) const> : __signature_base<_R(_Args...)> {};

template <class _R, class _C, class... _Args>
struct __signature_function<_R(_C::*)(_Args...) const &&> : __signature_base<_R(_Args...)> {};

template <class _T, class = void>
struct __signature_result_of {};

template <class _T>
struct __signature_result_of<_T,
  typename __signature_check<typename result_of<_T&&()>::type>::type>
    : __signature_base<typename result_of<_T&&()>::type()> {};

template <class _T, class = void>
struct __signature_class : __signature_result_of<_T> {};

template <class _T>
struct __signature_class<_T,
  typename __signature_check<decltype(&_T::operator())>::type>
    : __signature_function<decltype(&_T::operator())> {};

template <class _T>
struct __signature : conditional<is_class<_T>::value,
  __signature_class<_T>, __signature_function<_T>>::type {};

template <class _T>
using __signature_t = typename __signature<_T>::type;

// __make_signature: Creates a signature from return and argument types.

template <class... _Args>
struct __signature_arg_pack {};

template <class _Pack, class _Arg>
struct __signature_arg_pack_append;

template <class... _Args, class _Arg>
struct __signature_arg_pack_append<__signature_arg_pack<_Args...>, _Arg>
{
  typedef __signature_arg_pack<_Args..., _Arg> type;
};

template <class _R, class _Pack, class... _Args>
struct __signature_void_remover;

template <class _R, class... _Args>
struct __signature_void_remover<_R, __signature_arg_pack<_Args...>>
  : __signature_base<_R(_Args...)> {};

template <class _R, class _Pack, class _Arg, class... _Args>
struct __signature_void_remover<_R, _Pack, _Arg, _Args...>
  : __signature_void_remover<_R,
    typename __signature_arg_pack_append<_Pack, _Arg>::type, _Args...> {};

template <class _R, class _Pack, class... _Args>
struct __signature_void_remover<_R, _Pack, void, _Args...>
  : __signature_void_remover<_R, _Pack, _Args...> {};

template <class _R, class... _Args>
struct __make_signature
  : __signature_void_remover<_R, __signature_arg_pack<>, _Args...> {};

template <class _R>
struct __make_signature<_R, void> : __signature_base<_R()> {};

template <class _R, class... _Args>
struct __make_signature<_R, __signature_arg_pack<_Args...>>
  : __signature_base<_R(_Args...)> {};

template <class _R, class... _Args>
using __make_signature_t = typename __make_signature<_R, _Args...>::type;

// __signature_cat: Combines multiple signatures into one.

template <class... _Signatures>
struct __signature_cat;

template <class _R, class... _Args>
struct __signature_cat<_R(_Args...)>
  : __signature_base<_R(_Args...)> {};

template <class _R1, class... _Args1, class _R2, class... _Args2>
struct __signature_cat<_R1(_Args1...), _R2(_Args2...)>
  : __signature_base<_R1(_Args1..., _Args2...)> {};

template <class _R, class... _Args, class _Signature, class... _Signatures>
struct __signature_cat<_R(_Args...), _Signature, _Signatures...>
  : __signature_cat<typename __signature_cat<_R(_Args...), _Signature>::type, _Signatures...> {};

template <class... _Signatures>
using __signature_cat_t = typename __signature_cat<_Signatures...>::type;

// __result: Gets the result type of a signature.

template <class _Signature>
struct __result;

template <class _R, class... _Args>
struct __result<_R(_Args...)> { typedef _R type; };

template <class _Signature>
using __result_t = typename __result<_Signature>::type;

// __last_argument: Gets the type of the last argument in a signature.

template <class... _Args>
struct __last_argument_in_pack {};

template <>
struct __last_argument_in_pack<> {};

template <class _Arg>
struct __last_argument_in_pack<_Arg> { typedef _Arg type; };

template <class _Arg1, class _Arg2, class... _Args>
struct __last_argument_in_pack<_Arg1, _Arg2, _Args...>
  : __last_argument_in_pack<_Arg2, _Args...> {};

template <class _Signature>
struct __last_argument;

template <class _R, class... _Args>
struct __last_argument<_R(_Args...)> : __last_argument_in_pack<_Args...> {};

template <class _Signature>
using __last_argument_t = typename __last_argument<_Signature>::type;

// __function_continuation_of: Helper for determining the signature of a continuation.

template <class _T, class... _Args>
struct __function_continuation_of
{
  typedef __make_signature_t<void, typename result_of<_T(_Args...)>::type> signature;
};

// __chain: Chains a normal function to a continuation.

template <class _Func, class _FuncSignature, class _Continuation>
class __chain;

template <class _Func, class _FuncResult, class... _FuncArgs, class _Continuation>
class __chain<_Func, _FuncResult(_FuncArgs...), _Continuation>
{
public:
  template <class _F, class _C> __chain(_F&& __f, _C&& __c)
    : _M_func(forward<_F>(__f)), _M_continuation(forward<_C>(__c))
  {
  }

  void operator()(_FuncArgs... __args)
  {
    this->_Invoke(is_same<void, _FuncResult>(), forward<_FuncArgs>(__args)...);
  }

private:
  void _Invoke(true_type, _FuncArgs... __args)
  {
    std::move(_M_func)(forward<_FuncArgs>(__args)...);
    std::move(_M_continuation)();
  }

  void _Invoke(false_type, _FuncArgs... __args)
  {
    std::move(_M_continuation)(std::move(_M_func)(forward<_FuncArgs>(__args)...));
  }

  _Func _M_func;
  _Continuation _M_continuation;
};

} // inline namespace concurrency_v1
} // namespace experimental
} // namespace std

#endif
