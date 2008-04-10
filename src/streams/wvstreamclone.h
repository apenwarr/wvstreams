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
 * WvStreamClone does _not_ attempt to close the cloned stream in the
 * destructor.
 */
class WvStreamClone : public WvStream
{
public:
    /**
     * NOTE: we must NOT use *cloned at this point since the caller
     * may not have had a chance to initialize it yet!
     *
     * *cloned is still owned by the caller.
     */
    WvStreamClone(WvStream **_cloned)
        { cloned = _cloned; force_select(false, false, false); }
    virtual ~WvStreamClone();
    
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

protected:
    WvStream **cloned;
    WvStream *s() const
        { return cloned ? *cloned : (WvStream*)NULL; }
};

#endif // __WVSTREAMCLONE_H
