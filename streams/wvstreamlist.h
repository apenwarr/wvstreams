/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2000 Net Integration Technologies, Inc.
 * 
 * WvStreamList holds a list of WvStream objects -- and its select() and
 * callback() functions know how to handle multiple simultaneous streams.
 */
#ifndef __WVSTREAMLIST_H
#define __WVSTREAMLIST_H

#include "wvstream.h"

// Create the WvStreamListBase class - a simple linked list of WvStreams
DeclareWvList3(WvStream, WvStreamListBase, );

class WvStreamList : public WvStream, public WvStreamListBase
{
public:
    WvStreamList();
    virtual ~WvStreamList();
    virtual bool isok() const;
    virtual bool select_setup(SelectInfo &si);
    virtual bool test_set(SelectInfo &si);
    virtual void execute();
    
    bool auto_prune; // remove !isok() streams from the list automatically?
    
protected:
    WvStreamListBase sure_thing;
};

#endif // __WVSTREAMLIST_H
