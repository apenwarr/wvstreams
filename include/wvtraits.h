/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Provides useful type traits for generic programming.
 */
#ifndef __WVTRAITS_H
#define __WVTRAITS_H

#include <stdlib.h>

/**
 * WvTraits template declaration.
 * Partial template instantiations of this type provide
 * the following members:
 * 
 *   typename Param
 *       The type to use for passing T as a parameter.
 *   struct MemOps
 *       Contains members for managing large arrays of T.
 */
template<class T>
struct WvTraits;

/***** Built-in functors for performing block data manipulation *****/

/**
 * Provided functions:
 *   uninit_alloc : allocates an uninitialized array of objects
 *   uninit_free  : frees an uninitialized array of objects
 *   copy         : copies one initialized array of objects to another
 *   uninit_copy  : copies an initialized array of objects into an
 *                  uninitialized array of objects
 *   uninit       : uninitializes an initialized array of objects
 *   uninit_copy1 : copies an initialized object into uninitialized
 *                  storage for that object
 */
template<class T>
struct WvDeepMemOps
{
    inline static void *uninit_alloc(size_t count)
    {
        return T::operator new[](count * sizeof(count));
    }
    inline static void uninit_free(void *mem)
    {
        T::operator delete[](mem);
    }
    inline static void copy(T *dest, const T *src, size_t count)
    {
        for (int i = 0; i < count; ++i)
            dest[i] = src[i];
    }
    inline static void uninit_copy(void *dest, const T *src, size_t count)
    {
        for (int i = 0; i < count; ++i)
            new(& dest[i]) T(src[i]);
    }
    inline static void uninit_copy1(void *dest, const T &src)
    {
        new(dest) T(src);
    }
    inline static void uninit(T *dest, size_t count)
    {
        for (int i = 0; i < count; ++i)
            ~T(dest[i]);
    }
};
template<class T>
struct WvShallowMemOps
{
    inline static void *uninit_alloc(size_t count)
    {
        return T::operator new[](count * sizeof(count));
    }
    inline static void uninit_free(void *mem)
    {
        T::operator delete[](mem);
    }
    inline static void copy(T *dest, const T *src, size_t count)
    {
        memcpy(dest, src, count * sizeof(T));
    }
    inline static void uninit_copy(void *dest, const T *src, size_t count)
    {
        memcpy(dest, src, count * sizeof(T));
    }
    inline static void uninit_copy1(void *dest, const T src)
    {
        *(T*)dest = src;
    }
    inline static void uninit(T *dest, const T *src, size_t count)
    {
    }
};

/***** Actual type trait parameterisations *****/

// default for most types
template<class T>
struct WvTraits
{
    typedef WvDeepMemOps<T> MemOps;
    typedef const T& Param;
};
// pointer types
template<class T>
struct WvTraits<T*>
{
    typedef WvShallowMemOps<T*> MemOps;
    typedef const T Param;
};
// primitive types
#define Declare_PrimitiveWvTraits(T) \
struct WvTraits<T> \
{ \
    typedef WvShallowMemOps<T> MemOps; \
    typedef const T Param; \
}; 
Declare_PrimitiveWvTraits(unsigned char)
Declare_PrimitiveWvTraits(signed char)
Declare_PrimitiveWvTraits(unsigned short int)
Declare_PrimitiveWvTraits(signed short int)
Declare_PrimitiveWvTraits(unsigned int)
Declare_PrimitiveWvTraits(signed int)
Declare_PrimitiveWvTraits(unsigned long int)
Declare_PrimitiveWvTraits(signed long int)
Declare_PrimitiveWvTraits(unsigned long long int)
Declare_PrimitiveWvTraits(signed long long int)
Declare_PrimitiveWvTraits(double)
Declare_PrimitiveWvTraits(float)

#endif // __WVTRAITS_H
