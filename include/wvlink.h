/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * WvLink is one element of a linked list.  Used by wvlinklist.h, wvhashtable.h,
 * and so on.
 */
#ifndef __WVLINK_H
#define __WVLINK_H

#include <stdlib.h>  // for 'NULL'

// note: auto_free behaviour is a little bit weird; since WvLink does not
// know what data type it has received, there is no way it can call the
// right destructor.  So, the WvList needs to handle the data deletion
// by itself.  On the other hand, the auto_free flag needs to be stored in
// the WvLink.  <sigh>...
//
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
