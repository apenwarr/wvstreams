/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Totally fake XPLC bits, until the real XPLC gets released somehow.
 */ 
#ifndef __WVXPLC_H
#define __WVXPLC_H

#if HAS_XPLC

#include <xplc/xplc.h>
#include <xplc/utils.h>
#include <xplc/IServiceManager.h>

#else // not HAS_XPLC, so we'll fake it (badly)

struct UUID
{
    int a;
    short b, c;
    char d[8];
};

#define DEFINE_IID(iface, u1, u2, u3, u4, u5, u6, u7, u8, u9, u10, u11)

class IObject
{
public:
    unsigned int addRef() { return 1; }
    unsigned int release() { return 1; }
};

template<class T>
class GenericComponent : public T
{
};

template<class T, class T2> 
T *mutate(T2 *x) 
{
    return (T *)(void *)x;
}

#define UUID_MAP_BEGIN(name)
#define UUID_MAP_ENTRY(name)
#define UUID_MAP_END

#endif

#endif // __WVXPLC_H
