/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2000 Net Integration Technologies, Inc.
 * 
 * Implementation of the WvLoopback stream.  WvLoopback uses a
 * socketpair() to create a stream that allows you to read()
 * everything written to it, even (especially) across a fork() call.
 */
#include "wvloopback.h"
#include <sys/socket.h>
#include <fcntl.h>

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

    fcntl(rfd, F_SETFD, 1);
    fcntl(rfd, F_SETFL, O_RDONLY|O_NONBLOCK);
    fcntl(wfd, F_SETFD, 1);
    fcntl(wfd, F_SETFL, O_WRONLY|O_NONBLOCK);
}
