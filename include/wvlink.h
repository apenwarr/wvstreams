/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * WvLink is one element of a linked list.
 * Used by wvlinklist.h.
 */
#ifndef __WVLINK_H
#define __WVLINK_H

#include <stdlib.h>  // for 'NULL'

/**
 * WvLink is one elements of a WvList<T>.
 * <p>
 * Note that WvLink itself is untyped to minimize the amount of
 * generated code.  This means that WvLink cannot handle the
 * auto_free behaviour itself which would require static type
 * information.  Instead, it defers this behaviour to the
 * template instantiation of WvList<T> that uses it.
 * </p>
 */
class WvLink
{
public:
    void *data;
    WvLink *next;
    char *id;
    unsigned auto_free : 1;

    WvLink(void *_data, bool _auto_free, char *_id = NULL)
        { data = _data; next = NULL; auto_free = (unsigned)_auto_free;
	    id = _id; }

    WvLink(void *_data, WvLink *prev, WvLink *&tail, bool _auto_free,
	   char *_id = NULL);

    void unlink(WvLink *prev)
    {
	prev->next = next;
	delete this;
    }
};

#define WvIterStuff(_type_) \
	_type_ &operator () () const \
	    { return *ptr(); } \
	_type_ *operator -> () const \
	    { return ptr(); } \
        _type_ &operator* () const \
            { return *ptr(); }

#endif // __WVLINK_H
