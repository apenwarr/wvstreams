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
    
    bool operator== (const UUID &other) const
        { return !memcmp(this, &other, sizeof(*this)); }
};

template<class T>
struct IID {
};

#define DEFINE_IID(iface, u1, u2, u3, u4, u5, u6, u7, u8, u9, u10, u11) \
static const UUID iface##_IID = u1, u2, u3, u4, u5, u6, u7, u8, u9, u10, u11; \
struct IID<iface> { \
  static const UUID &get() { \
    return iface##_IID; \
  } \
}


class IObject
{
public:
    virtual ~IObject() { };
    
    virtual unsigned int addRef() = 0;
    virtual unsigned int release() = 0;
};

DEFINE_IID(IObject, {0x12345678, 0xb653, 0x43d7,
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

#define UUID_MAP_BEGIN(name)
#define UUID_MAP_ENTRY(name)
#define UUID_MAP_END

#endif

#endif // __WVXPLC_H
