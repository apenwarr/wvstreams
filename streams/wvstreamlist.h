/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997, 1998 Worldvisions Computer Technology, Inc.
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
    virtual bool isok() const;
    virtual bool select_setup(fd_set &r, fd_set &w, fd_set &x, int &max_fd,
			      bool readable, bool writable, bool isexception);
    virtual bool test_set(fd_set &r, fd_set &w, fd_set &x);
    
protected:
    fd_set sel_r, sel_w, sel_x;
    WvStreamListBase sure_thing;
    static Callback dist_callback;
};

#endif // __WVSTREAMLIST_H
