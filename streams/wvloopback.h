/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */ 
#ifndef __WVLOOPBACK_H
#define __WVLOOPBACK_H

#include "wvsplitstream.h"

class WvSplitStream;

/**
 * Implementation of a WvLoopback stream.  WvLoopback uses a
 * socketpair() to create a stream that allows you to read()
 * everything written to it, even (especially) across a fork() call.
 */
class WvLoopback : public WvSplitStream
{
public:
    WvLoopback();
};

#endif // __WVLOOPBACK_H
