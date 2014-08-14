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
inline namespace concurrency_v1 {

template <unsigned char __max = UCHAR_MAX>
class __small_block_recycler
{
public:
  __small_block_recycler(const __small_block_recycler&) = delete;
  __small_block_recycler& operator=(const __small_block_recycler&) = delete;

  __small_block_recycler() : _M_memory(nullptr), _M_next_memory(nullptr)
  {
  }

// The clang compiler shipped with Xcode doesn't support the thread_local
// keyword. We make do with the __thread keyword extension, but this means we
// cannot have a non-trivial destructor.
#if !defined(__APPLE__) || !defined(__clang__)
  ~__small_block_recycler()
  {
    if (_M_memory)
      ::operator delete(_M_memory);
    if (_M_next_memory)
      ::operator delete(_M_next_memory);
  }
#endif

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
      _M_memory = _M_next_memory;
      _M_next_memory = nullptr;

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

        if (_M_next_memory == 0)
        {
          unsigned char* const __mem = static_cast<unsigned char*>(__p);
          __mem[0] = __mem[__size];
          _M_next_memory = __p;
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
  void* _M_next_memory;
#if defined(__APPLE__) && defined(__clang__)
  static __thread __small_block_recycler _S_instance;
#else
  static thread_local __small_block_recycler _S_instance;
#endif
};

#if defined(__APPLE__) && defined(__clang__)
template <unsigned char __max>
__thread __small_block_recycler<__max> __small_block_recycler<__max>::_S_instance;
#else
template <unsigned char __max>
thread_local __small_block_recycler<__max> __small_block_recycler<__max>::_S_instance;
#endif

template <class _T>
class __small_block_allocator
{
public:
  typedef _T* pointer;
  typedef const _T* const_pointer;
  typedef _T value_type;

  template <class _U>
  struct rebind
  {
    typedef __small_block_allocator<_U> other;
  };

  __small_block_allocator()
  {
  }

  template <class _U>
  __small_block_allocator(const __small_block_allocator<_U>&)
  {
  }

  template <class _U>
  __small_block_allocator(const allocator<_U>&)
  {
  }

  _T* allocate(std::size_t __n)
  {
    return static_cast<_T*>(__small_block_recycler<>::_Instance()._Allocate(__n * sizeof(_T)));
  }

  void deallocate(_T* __p, std::size_t __n)
  {
    __small_block_recycler<>::_Instance()._Deallocate(__p, __n * sizeof(_T));
  }
};

template <>
class __small_block_allocator<void>
{
public:
  typedef void* pointer;
  typedef const void* const_pointer;
  typedef void value_type;

  template <class _U>
  struct rebind
  {
    typedef __small_block_allocator<_U> other;
  };

  __small_block_allocator()
  {
  }

  template <class _U>
  __small_block_allocator(const __small_block_allocator<_U>&)
  {
  }

  template <class _U>
  __small_block_allocator(const allocator<_U>&)
  {
  }
};

template <class _Allocator, class _U>
struct __small_block_rebind
{
  typedef typename _Allocator::template rebind<_U>::other _Type;
};

template <class _T, class _U>
struct __small_block_rebind<allocator<_T>, _U>
{
  typedef __small_block_allocator<_U> _Type;
};

template <class _Allocator, class _U>
using __small_block_rebind_t = typename __small_block_rebind<_Allocator, _U>::_Type;

} // inline namespace concurrency_v1
} // namespace experimental
} // namespace std

#endif
