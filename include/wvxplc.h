/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Totally fake XPLC bits, until the real XPLC gets released somehow.
 */ 
#ifndef __WVXPLC_H
#define __WVXPLC_H

#include "wvautoconf.h"

#if HAVE_LIBXPLC

// wvstreams uses 'X' in front of these words, to avoid conflicts when
// compiling windows apps.
#define XUUID UUID
#define XIID IID
#define DEFINE_XIID DEFINE_IID

#define XUUID_MAP_BEGIN UUID_MAP_BEGIN
#define XUUID_MAP_ENTRY UUID_MAP_ENTRY
#define XUUID_MAP_END   UUID_MAP_END

#include <xplc/xplc.h>
#include <xplc/utils.h>
#include <xplc/IServiceManager.h>

#else // not HAVE_LIBXPLC, so we'll fake it (badly)

#include <string.h> // for memcmp

struct XUUID
{
    unsigned int a;
    unsigned short b, c;
    unsigned char d[8];
    
    bool operator== (const XUUID &other) const
        { return !memcmp(this, &other, sizeof(*this)); }
};

template<class T>
struct XIID {
};

#define DEFINE_XIID(iface, u1, u2, u3, u4, u5, u6, u7, u8, u9, u10, u11) \
static const XUUID iface##_XIID = u1, u2, u3, u4, u5, u6, u7, u8, u9, u10, u11; \
template<> \
struct XIID<iface> { \
  static const XUUID &get() { \
    return iface##_XIID; \
  } \
}


class IObject
{
public:
    virtual ~IObject() { };
    
    virtual unsigned int addRef() = 0;
    virtual unsigned int release() = 0;
};

DEFINE_XIID(IObject, {0x12345678, 0xb653, 0x43d7,
  {0xb0, 0x56, 0x8b, 0x9d, 0xde, 0x9a, 0xbe, 0x9d}});



template<class T>
class GenericComponent : public T
{
private:
    unsigned refcount;
    
public:
    GenericComponent() { refcount = 0; }
    
    virtual unsigned int addRef()
        { return ++refcount; }
    
    virtual unsigned int release()
    { 
	if (--refcount)
	    return refcount;
	
	refcount = 1;
	delete this;
	return 0;
    }
};

template<class T, class T2> 
T *mutate(T2 *x) 
{
    return (T *)(void *)x;
}

#define XUUID_MAP_BEGIN(name)
#define XUUID_MAP_ENTRY(name)
#define XUUID_MAP_END

#endif // HAVE_LIBXPLC

#endif // __WVXPLC_H
