/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997, 1998 Worldvisions Computer Technology, Inc.
 * 
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
#ifndef __WVSTREAMCLONE_H
#define __WVSTREAMCLONE_H

#include "wvstream.h"

class WvStreamClone : public WvStream
{
public:
    // NOTE: we must NOT use *cloned at this point since the caller may
    //  not have had a chance to initialize it yet!
    WvStreamClone(WvStream **_cloned)
        { cloned = _cloned; setcallback(default_callback, this); }
    virtual ~WvStreamClone();
    
    virtual void close();
    virtual int getfd() const;
    virtual size_t uread(void *buf, size_t size);
    virtual size_t uwrite(const void *buf, size_t size);
    virtual bool test_set(fd_set &r, fd_set &w, fd_set &x);
    virtual bool isok() const;
    virtual int geterr() const;
    virtual const char *errstr() const;
    virtual bool select_setup(fd_set &r, fd_set &w, fd_set &x, int &max_fd,
			      bool readable, bool writable, bool isexception);
    virtual const WvAddr *src() const;

protected:
    WvStream **cloned;
    WvStream *s() const
        { return cloned ? *cloned : (WvStream*)NULL; }
    static WvStream::Callback default_callback;
};

#endif // __WVSTREAMCLONE_H
