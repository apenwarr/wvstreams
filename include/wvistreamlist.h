/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * FIXME: Suspiciously similar to wvstreamlist, and with tons of duplicated
 * code.  Blech.
 */ 
#ifndef __WVISTREAMLIST_H
#define __WVISTREAMLIST_H

#include "wvstream.h"

/**
 * Create the WvStreamListBase class - a simple linked list of WvStreams
 */
DeclareWvList3(IWvStream, WvIStreamListBase, );

/**
 * WvStreamList holds a list of WvStream objects -- and its select() and
 * callback() functions know how to handle multiple simultaneous streams.
 */
class WvIStreamList : public WvStream, public WvIStreamListBase
{
public:
    WvIStreamList();
    virtual ~WvIStreamList();
    virtual bool isok() const;
    virtual bool pre_select(SelectInfo &si);
    virtual bool post_select(SelectInfo &si);
    virtual void execute();
    
    void unlink(WvStream *data)
        { sure_thing.unlink(data); WvIStreamListBase::unlink(data); }
    
    bool auto_prune; // remove !isok() streams from the list automatically?
    static WvIStreamList globallist;
    
protected:
    WvIStreamListBase sure_thing;
};

#endif // __WVISTREAMLIST_H
