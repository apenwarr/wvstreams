/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Implementation of a two-way version of WvLoopback.  See wvloopback2.h.
 */
#include "wvloopback2.h"
#include <fcntl.h>

#ifndef _WIN32
# include <sys/socket.h>
#else
# include <io.h>
#endif

#ifdef _WIN32
int socketpair(int family, int type, int protocol, int *sb);
#endif

void wv_loopback2(IWvStream *&s1, IWvStream *&s2)
{
    int socks[2];
    
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, socks))
    {
	int errnum = errno;
	s1 = new WvStream;
	s2 = new WvStream;
	s1->seterr(errnum);
	s2->seterr(errnum);
	return;
    }
    
#ifndef _WIN32
    fcntl(socks[0], F_SETFD, 1);
    fcntl(socks[0], F_SETFL, O_RDONLY|O_NONBLOCK);
    fcntl(socks[1], F_SETFD, 1);
    fcntl(socks[1], F_SETFL, O_WRONLY|O_NONBLOCK);
#else
    u_long arg = 1;
    ioctlsocket(socks[0], FIONBIO, &arg); // non-blocking
    ioctlsocket(socks[1], FIONBIO, &arg); // non-blocking
#endif
    
    s1 = new WvFdStream(socks[0], socks[0]);
    s2 = new WvFdStream(socks[1], socks[1]);
}
