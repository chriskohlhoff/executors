//
// small_block_recycler.h
// ~~~~~~~~~~~~~~~~~~~~~~
// Recycles small allocations that may be associated with a chain of operations.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_SMALL_BLOCK_RECYCLER_H
#define EXECUTORS_EXPERIMENTAL_BITS_SMALL_BLOCK_RECYCLER_H

#include <climits>
#include <memory>
#include <utility>

namespace std {
namespace experimental {

template <unsigned char __max = UCHAR_MAX>
class __small_block_recycler
{
public:
  __small_block_recycler(const __small_block_recycler&) = delete;
  __small_block_recycler& operator=(const __small_block_recycler&) = delete;

  __small_block_recycler() : _M_memory(nullptr)
  {
  }

  ~__small_block_recycler()
  {
    if (_M_memory)
      ::operator delete(_M_memory);
  }

  template <class _T>
  struct _Delete
  {
    constexpr _Delete() noexcept {}
    template <class _U> _Delete(const _Delete<_U>&) {}
    void operator()(_T* __p) const { _S_instance._Destroy(__p); }
  };

  template <class _T>
  using _Unique_ptr = unique_ptr<_T, _Delete<_T>>;

  template <class _T, class... _Args>
  static _T* _Create(_Args&&... __args)
  {
    _Init __i;
    __i._M_memory = __i._M_instance._Allocate(sizeof(_T));
    __i._M_size = sizeof(_T);
    _T* __p = new (__i._M_memory) _T(forward<_Args>(__args)...);
    __i._M_memory = nullptr;
    return __p;
  }

  template <class _T>
  static void _Destroy(_T* __p)
  {
    __p->~_T();
    _S_instance._Deallocate(__p, sizeof(_T));
  }

  void* _Allocate(size_t __size)
  {
    if (_M_memory)
    {
      void* const __p = _M_memory;
      _M_memory = 0;

      unsigned char* const __mem = static_cast<unsigned char*>(__p);
      if (static_cast<size_t>(__mem[0]) >= __size)
      {
        __mem[__size] = __mem[0];
        return __p;
      }

      ::operator delete(__p);
    }

    void* const __p = ::operator new(__size + 1);
    unsigned char* const __mem = static_cast<unsigned char*>(__p);
    __mem[__size] = (__size <= __max) ? static_cast<unsigned char>(__size) : 0;
    return __p;
  }

  void _Deallocate(void* __p, size_t __size)
  {
    if (__p)
    {
      if (__size <= __max)
      {
        if (_M_memory == 0)
        {
          unsigned char* const __mem = static_cast<unsigned char*>(__p);
          __mem[0] = __mem[__size];
          _M_memory = __p;
          return;
        }
      }

      ::operator delete(__p);
    }
  }

  static __small_block_recycler& _Instance()
  {
    return _S_instance;
  }

private:
  struct _Init
  {
    __small_block_recycler& _M_instance = _S_instance;
    void* _M_memory = nullptr;
    size_t _M_size = 0;
    ~_Init() { _M_instance._Deallocate(_M_memory, _M_size); }
  };

  void* _M_memory;
  static thread_local __small_block_recycler _S_instance;
};

template <unsigned char __max>
thread_local __small_block_recycler<__max> __small_block_recycler<__max>::_S_instance;

} // namespace experimental
} // namespace std

#endif
