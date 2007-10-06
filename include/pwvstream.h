/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */ 
#ifndef __PWVSTREAM_H
#define __PWVSTREAM_H

#include "iwvstream.h"
#include "wvstream.h"      // FIXME: don't include this!
#include "wvstreamclone.h" // FIXME: don't include this!
#include "wvmoniker.h"
#include "wvcallback.h"
#include <tr1/memory>

/**
 * PWvStream is a smart pointer to an IWvStream object.  It is designed for
 * maximum ABI-stability.  Even though individual WvStream-derived classes
 * might change their size and object layout, if you create, destroy, and
 * access them using PWvStream and IWvStream, you should be safe.
 * 
 * Note that this class is entirely inlined.  PWvStream's object layout may
 * change at any time, so you shouldn't pass it around between modules; pass
 * around IWvStream objects instead (perhaps from PWvStream::addRef()).  That
 * way people using two different versions of PWvStream will still be able
 * to interoperate.
 * 
 * FIXME:
 * PWvStream is incomplete, so it does not yet give the required level of ABI
 * stability.  Things to do:
 * 
 *  - Remove all references to classes, using only interfaces.  That means
 *    no WvStream or WvStreamClone, only IWvStream.  IWvStream might have
 *    to change (or introduce a new IWvBufStream) to make this work.
 * 
 *  - Definitely don't rely on C++ RTTI here.
 * 
 *  - Add a way to get errors back when moniker construction fails.  This
 *    is relatively easy; just make the returned stream a null: stream and
 *    do seterr() on it.
 */
class PWvStream : public std::tr1::shared_ptr<WvStream>
{
    static WvStream *clean_stream(IWvStream *s)
    {
	WvStream *ss = dynamic_cast<WvStream *>(s);
	if (ss)
	    return ss;
	else
	    return new WvStreamClone(s);
    }
    
    static WvStream *make_stream(WvStringParm moniker, IObject *obj)
    {
	IWvStream *s = wvcreate<IWvStream>(moniker, obj);
	if (!s)
	    s = wvcreate<IWvStream>("null:");
	assert(s != NULL);
	return clean_stream(s);
    }
    
public:
    PWvStream()
    {
	// Pointer points to NULL right now, but it could be reassigned
	// later using operator=().
    }
    
    PWvStream(IWvStream *s)
	: std::tr1::shared_ptr<WvStream>(clean_stream(s),
				  wv::bind(&IWvStream::release, wv::_1))
    {
    }
    
    PWvStream(WvStringParm moniker, IObject *obj = 0)
	: std::tr1::shared_ptr<WvStream>(make_stream(moniker, obj),
				  wv::bind(&IWvStream::release, wv::_1))
    {
	// Note: pointer is definitely not NULL here, because make_stream is
	// careful.
    }
    
    WvStream *addRef() const
    {
	if (get())
	    get()->addRef();
	return get();
    }
};

#endif // __PWVSTREAM_H
