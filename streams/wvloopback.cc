/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997, 1998, 1999 Worldvisions Computer Technology, Inc.
 * 
 * Implementation of the WvLoopback stream.  WvLoopback uses a
 * socketpair() to create a stream that allows you to read()
 * everything written to it, even (especially) across a fork() call.
 */
#include "wvloopback.h"
#include <sys/socket.h>

WvLoopback::WvLoopback()
{
    int socks[2];
    
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, socks))
    {
	errnum = errno;
	return;
    }
    
    rfd = socks[0];
    wfd = socks[1];
}
