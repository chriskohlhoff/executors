//
// execution_context.h
// ~~~~~~~~~~~~~~~~~~~
// Execution context where functions will be invoked.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_EXECUTION_CONTEXT_H
#define EXECUTORS_EXPERIMENTAL_BITS_EXECUTION_CONTEXT_H

#include <memory>
#include <stdexcept>
#include <type_traits>

namespace std {
namespace experimental {

inline execution_context::execution_context()
  : _M_first_service(nullptr)
{
}

inline execution_context::~execution_context()
{
  while (_M_first_service)
  {
    service* __s = _M_first_service->_M_next;
    delete _M_first_service;
    _M_first_service = __s;
  }
}

inline void execution_context::shutdown()
{
  _M_mutex.lock();
  execution_context::service* const __first = _M_first_service;
  _M_mutex.unlock();

  service* __s = __first;
  while (__s)
  {
    __s->shutdown_service();
    __s = __s->_M_next;
  }
}

inline execution_context::service::service(execution_context& __c)
  : _M_context(__c), _M_next(nullptr)
{
}

inline execution_context::service::~service()
{
}

inline execution_context& execution_context::service::context()
{
  return _M_context;
}

class service_already_exists
  : logic_error
{
public:
  service_already_exists()
    : logic_error("Service already_exists.")
  {
  }
};

template <class _Service>
inline _Service* __create_service(execution_context& __c, true_type)
{
  return new _Service(__c);
};

template <class _Service>
inline _Service* __create_service(execution_context&, false_type)
{
  throw bad_cast();
};

template <class _Service>
struct __service_void_type
{
  typedef void _Type;
};

template <class _Service, class = void>
struct __service_key_type
{
  typedef _Service _Type;
};

template <class _Service>
struct __service_key_type<_Service,
  typename __service_void_type<typename _Service::key_type>::_Type>
{
  typedef typename _Service::key_type _Type;
};

template <class _Service>
using __service_key = typename __service_key_type<_Service>::_Type;

template <class _Service> _Service& use_service(execution_context& __c)
{
  unique_lock<mutex> __lock(__c._M_mutex);
  execution_context::service* const __first = __c._M_first_service;
  __lock.unlock();

  // Check if service already exists.
  const type_info* __id = &typeid(__service_key<_Service>);
  execution_context::service* __s = __first;
  while (__s)
  {
    if (*__id == *__s->_M_id)
      return *static_cast<_Service*>(__s);
    __s = __s->_M_next;
  }

  // Creation is performed without holding lock, to allow reentrancy.
  unique_ptr<execution_context::service> __new_s(
    __create_service<_Service>(__c,
      is_constructible<_Service, execution_context&>()));
  __new_s->_M_id = __id;

  // Check if a duplicate was created while lock was not held.
  __lock.lock();
  __s = __c._M_first_service;
  while (__s != __first)
  {
    if (*__id == *__s->_M_id)
      return *static_cast<_Service*>(__s);
    __s = __s->_M_next;
  }

  // Take ownership and return.
  __new_s->_M_next = __c._M_first_service;
  __c._M_first_service = __new_s.release();
  return *static_cast<_Service*>(__c._M_first_service);
}

template <class _Service, class... _Args> _Service&
  make_service(execution_context& __c, _Args&&... __args)
{
  unique_lock<mutex> __lock(__c._M_mutex);
  execution_context::service* const __first = __c._M_first_service;
  __lock.unlock();

  // Check if service already exists.
  const type_info* __id = &typeid(__service_key<_Service>);
  execution_context::service* __s = __first;
  while (__s)
  {
    if (*__id == *__s->_M_id)
      throw service_already_exists();
    __s = __s->_M_next;
  }

  // Creation is performed without holding lock, to allow reentrancy.
  unique_ptr<execution_context::service> __new_s(
    new _Service(__c, forward<_Args>(__args)...));
  __new_s->_M_id = __id;

  // Check if a duplicate was created while lock was not held.
  __lock.lock();
  __s = __c._M_first_service;
  while (__s != __first)
  {
    if (*__id == *__s->_M_id)
      throw service_already_exists();
    __s = __s->_M_next;
  }

  // Take ownership and return.
  __new_s->_M_next = __c._M_first_service;
  __c._M_first_service = __new_s.release();
  return *static_cast<_Service*>(__c._M_first_service);
}

template <class _Service> bool has_service(execution_context& __c) noexcept
{
  __c._M_mutex.lock();
  execution_context::service* const __first = __c._M_first_service;
  __c._M_mutex.unlock();

  const type_info* __id = &typeid(__service_key<_Service>);
  execution_context::service* __s = __first;
  while (__s)
  {
    if (*__id == *__s->_M_id)
      return true;
    __s = __s->_M_next;
  }

  return false;
}

} // namespace experimental
} // namespace std

#endif
