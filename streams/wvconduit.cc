/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Implementation of the WvConduit stream.  WvConduit uses a
 * socketpair() to create a pair of streams linked in a way that
 * allows you to read from one whatever is written to the other.
 */

#include "wvconduit.h"
#include <sys/socket.h>
#include <fcntl.h>

WvConduit::WvConduit() : slave(false)
{
    int socks[2];

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, socks))
    {
	errnum = errno;
	return;
    }

    fcntl(socks[0], F_SETFD, 1);
    fcntl(socks[0], F_SETFL, O_RDONLY|O_NONBLOCK);
    fcntl(socks[1], F_SETFD, 1);
    fcntl(socks[1], F_SETFL, O_WRONLY|O_NONBLOCK);

    wfd = rfd = socks[0];

    other = new WvConduit(this, socks[1]);
}

WvConduit::WvConduit(WvConduit *master, int fd) 
    : WvFDStream(fd, fd), other(master), slave(true)
{
}

WvConduit *WvConduit::get_slave()
{
    return other;
}

void WvConduit::shutdown()
{
    ::shutdown(wfd, SHUT_WR);
}

WvConduit::~WvConduit()
{
    if (slave)
        other->other = NULL;
    else
        delete other;
}
