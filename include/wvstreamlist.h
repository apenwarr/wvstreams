/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */

/** \file
 * Provides support for managing a list of WvStreams.
 */ 
#ifndef __WVSTREAMLIST_H
#define __WVSTREAMLIST_H

#include "wvstream.h"

/**
 * Create the WvStreamListBase class - a simple linked list of WvStreams
 */
DeclareWvList3(WvStream, WvStreamListBase, );

/**
 * WvStreamList holds a list of WvStream objects -- and its select() and
 * callback() functions know how to handle multiple simultaneous streams.
 */
class WvStreamList : public WvStream, public WvStreamListBase
{
public:
    WvStreamList();
    virtual ~WvStreamList();
    virtual bool isok() const;
    virtual bool pre_select(SelectInfo &si);
    virtual bool post_select(SelectInfo &si);
    virtual void execute();
    
    void unlink(WvStream *data)
        { sure_thing.unlink(data); WvStreamListBase::unlink(data); }
    
    bool auto_prune; // remove !isok() streams from the list automatically?
    
protected:
    WvStreamListBase sure_thing;
};

#endif // __WVSTREAMLIST_H
