/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Provides support for constructing streams that wrap other streams.
 * This can turn a (relatively useless) IWvStream into a useful WvStream.
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
     *
     * If you want to be certain that the cloned stream will not be
     * harmed in any way by deleting or closing the wrapper stream,
     * set disassociate_on_close to true.
     */
    WvStreamClone(IWvStream *_cloned = NULL);
    virtual ~WvStreamClone();

    IWvStream *cloned;
    bool disassociate_on_close; // defaults to false
    
    virtual void setclone(IWvStream *clone);
    virtual void close();
    virtual bool flush_internal(time_t msec_timeout);
    virtual size_t uread(void *buf, size_t size);
    virtual size_t uwrite(const void *buf, size_t size);
    virtual bool isok() const;
    virtual int geterr() const;
    virtual WvString errstr() const;
    virtual void pre_select(SelectInfo &si);
    virtual bool post_select(SelectInfo &si);
    virtual const WvAddr *src() const;
    virtual void execute();
    virtual void noread();
    virtual void nowrite();

private:
    void close_callback(WvStream &s);

protected:
    WvString my_type;
public:
    const char *wstype() const { return my_type; }
};

#endif // __WVSTREAMCLONE_H
