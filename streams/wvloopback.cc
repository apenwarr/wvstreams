/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Implementation of the WvLoopback stream.  WvLoopback uses a
 * socketpair() to create a stream that allows you to read()
 * everything written to it, even (especially) across a fork() call.
 */
#include "wvloopback.h"

#ifndef _WIN32
#include <sys/socket.h>
#else
#include <io.h>
#endif
#include <fcntl.h>

#ifdef _WIN32
int socketpair (int family, int type, int protocol, int *sb);
#endif

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

#ifndef _WIN32
    fcntl(rfd, F_SETFD, 1);
    fcntl(rfd, F_SETFL, O_RDONLY|O_NONBLOCK);
    fcntl(wfd, F_SETFD, 1);
    fcntl(wfd, F_SETFL, O_WRONLY|O_NONBLOCK);
#else
    u_long arg = 1;
    ioctlsocket(rfd, FIONBIO, &arg); // non-blocking
    ioctlsocket(wfd, FIONBIO, &arg); // non-blocking
#endif
}
