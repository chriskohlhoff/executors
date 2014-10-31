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

template <class _Purpose = void>
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
    __mem[__size] = (__size <= UCHAR_MAX) ? static_cast<unsigned char>(__size) : 0;
    return __p;
  }

  void _Deallocate(void* __p, size_t __size)
  {
    if (__p)
    {
      if (__size <= UCHAR_MAX)
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
#if defined(__APPLE__) && defined(__clang__)
    return _S_instance;
#elif defined(_MSC_VER) || defined(__GNUC__)
    if (!_S_instance)
      _S_instance = new __small_block_recycler;
    return *_S_instance;
#else
    return _S_instance;
#endif
  }

private:
  void* _M_memory;
  void* _M_next_memory;
#if defined(__APPLE__) && defined(__clang__)
  static __thread __small_block_recycler _S_instance;
#elif defined(_MSC_VER)
  static __declspec(thread) __small_block_recycler* _S_instance;
#elif defined(__GNUC__)
  static thread_local __small_block_recycler* _S_instance;
#else
  static thread_local __small_block_recycler _S_instance;
#endif
};

#if defined(__APPLE__) && defined(__clang__)
template <class _Purpose>
__thread __small_block_recycler<_Purpose> __small_block_recycler<_Purpose>::_S_instance;
#elif defined(_MSC_VER)
template <class _Purpose>
__declspec(thread) __small_block_recycler<_Purpose>* __small_block_recycler<_Purpose>::_S_instance;
#elif defined(__GNUC__)
template <class _Purpose>
thread_local __small_block_recycler<_Purpose>* __small_block_recycler<_Purpose>::_S_instance;
#else
template <class _Purpose>
thread_local __small_block_recycler<_Purpose> __small_block_recycler<_Purpose>::_S_instance;
#endif

template <class _T, class _Purpose = void>
class __small_block_allocator
{
public:
  typedef _T* pointer;
  typedef const _T* const_pointer;
  typedef _T value_type;

  template <class _U>
  struct rebind
  {
    typedef __small_block_allocator<_U, _Purpose> other;
  };

  __small_block_allocator()
  {
  }

  template <class _U>
  __small_block_allocator(const __small_block_allocator<_U, _Purpose>&)
  {
  }

  template <class _U>
  __small_block_allocator(const allocator<_U>&)
  {
  }

  _T* allocate(std::size_t __n)
  {
    return static_cast<_T*>(__small_block_recycler<_Purpose>::_Instance()._Allocate(__n * sizeof(_T)));
  }

  void deallocate(_T* __p, std::size_t __n)
  {
    __small_block_recycler<_Purpose>::_Instance()._Deallocate(__p, __n * sizeof(_T));
  }
};

template <class _Purpose>
class __small_block_allocator<void, _Purpose>
{
public:
  typedef void* pointer;
  typedef const void* const_pointer;
  typedef void value_type;

  template <class _U>
  struct rebind
  {
    typedef __small_block_allocator<_U, _Purpose> other;
  };

  __small_block_allocator()
  {
  }

  template <class _U>
  __small_block_allocator(const __small_block_allocator<_U, _Purpose>&)
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

template <class _Allocator, class _T>
class __small_block_delete
{
public:
  explicit __small_block_delete(const _Allocator& __a) noexcept
    : _M_alloc(__a)
  {
  }

  explicit __small_block_delete(const __small_block_rebind_t<_Allocator, _T>& __a) noexcept
    : _M_alloc(__a)
  {
  }

  template <class _OtherAllocator, class _U>
  __small_block_delete(const __small_block_delete<_OtherAllocator, _U>& __d) noexcept
    : _M_alloc(__d._M_alloc)
  {
  }

  void operator()(_T* __p) const
  {
    __p->~_T();
    _M_alloc.deallocate(__p, 1);
  }

private:
  mutable __small_block_rebind_t<_Allocator, _T> _M_alloc;
};

template <class _Allocator, class _T>
using __small_block_ptr = unique_ptr<_T, __small_block_delete<_Allocator, _T>>;

template <class _T, class _Allocator, class... _Args>
__small_block_ptr<_Allocator, _T> _Allocate_small_block(const _Allocator& __alloc, _Args&&... __args)
{
  __small_block_rebind_t<_Allocator, _T> __rebound_alloc(__alloc);
  _T* __raw_p = __rebound_alloc.allocate(1);
  try
  {
    _T* __p =  new (__raw_p) _T(forward<_Args>(__args)...);
    return __small_block_ptr<_Allocator, _T>(__p, __small_block_delete<_Allocator, _T>(__rebound_alloc));
  }
  catch (...)
  {
    __rebound_alloc.deallocate(__raw_p, 1);
    throw;
  }
}

template <class _T, class _Allocator>
inline __small_block_ptr<_Allocator, _T> _Adopt_small_block(const _Allocator& __alloc, _T* __p) noexcept
{
  return __small_block_ptr<_Allocator, _T>(__p, __small_block_delete<_Allocator, _T>(__alloc));
}

} // inline namespace concurrency_v1
} // namespace experimental
} // namespace std

#endif
