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

WvConduit::WvConduit()
{
    int rs[2];
    int ws[2];

    setup_sockpair(rs);
    setup_sockpair(ws);

    rfd = rs[0];
    wfd = ws[0];

    slave = new WvFDStream(rs[1], ws[1]);
}

void WvConduit::setup_sockpair(int *socks)
{
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

WvStream *WvConduit::get_slave()
{
    return slave;
}

WvConduit::~WvConduit()
{
    delete slave;
}
