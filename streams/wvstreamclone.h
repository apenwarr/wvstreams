/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */ 
#ifndef __WVSTREAMCLONE_H
#define __WVSTREAMCLONE_H

#include "wvstream.h"

/**
 * WvStreamClone simply forwards all requests to the "cloned" stream.
 * 
 * A class derived from WvStreamClone can contain a WvStream as a
 * dynamically allocated data member, but act like the stream itself.
 * 
 * This is useful for classes that need to create/destroy WvPipes
 * while they run, for example, yet do not want users to know about
 * the member variable.
 * 
 * WvStreamClone _does_ attempt to close the cloned stream in the
 * destructor.
 */
class WvStreamClone : public WvStream
{
public:
    /**
     * WvStreamClone gains ownership (i.e. it will delete it when it
     * dies) of the stream you give it. If you do not want that to
     * happen, set cloned to NULL before destroying the WvStreamClone
     * (for example, in your destructor if you derive WvStreamClone).
     */
    WvStreamClone(WvStream *_cloned):
	cloned(_cloned)
        { force_select(false, false, false); }
    virtual ~WvStreamClone();

    WvStream *cloned;
    
    virtual void close();
    virtual int getrfd() const;
    virtual int getwfd() const;
    virtual size_t uread(void *buf, size_t size);
    virtual size_t uwrite(const void *buf, size_t size);
    virtual bool isok() const;
    virtual int geterr() const;
    virtual const char *errstr() const;
    virtual bool pre_select(SelectInfo &si);
    virtual bool post_select(SelectInfo &si);
    virtual const WvAddr *src() const;
    virtual void execute();
};

#endif // __WVSTREAMCLONE_H
