/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2004 Net Integration Technologies, Inc.
 *
 * Contains code you'd rather not think about.
 */
#ifndef __WVMAGIC_H
#define __WVMAGIC_H

#include <xplc/IObject.h>

template<class T, bool b>
struct Magic_Helper
{
    static void maybe_addref(T* obj)
    {}
    static void destroy(T* obj)
    {
	delete obj;
    }
};


template<class T>
struct Magic_Helper<T, true>
{
    static void maybe_addref(T* obj)
    {
	obj->addRef();
    }
    static void destroy(T* obj)
    {
	obj->release();
    }
};


template<class From>
class Magic
{
    typedef char Yes;
    struct No { char dummy[2]; };
    static From* from;
    static Yes test(IObject*);
    static No test(...);
public:
    static void maybe_addref(From* obj)
    {
	Magic_Helper<From, (sizeof(test(from)) == sizeof(Yes))>::maybe_addref(obj);
    }
    static void destroy(From* obj)
    {
	Magic_Helper<From, (sizeof(test(from)) == sizeof(Yes))>::destroy(obj);
    }
};


#endif /* __WVMAGIC_H */
