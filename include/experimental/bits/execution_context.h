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

#include <stdexcept>

namespace std {
namespace experimental {

inline execution_context::execution_context()
{
}

inline execution_context::~execution_context()
{
}

inline void execution_context::shutdown()
{
}

inline execution_context::id::id()
{
}

inline execution_context::service::service(execution_context& __c)
  : _M_context(__c), _M_next(0)
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

template <class _Service> _Service& use_service(execution_context& __c)
{
  throw std::bad_cast();
}

template <class _Service, class... _Args> _Service&
  make_service(execution_context& __c, _Args&&... __args)
{
  throw std::bad_cast();
}

template <class _Service> bool has_service(execution_context& __c)
{
  return false;
}

} // namespace experimental
} // namespace std

#endif
