/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1998 Worldvisions Computer Technology, Inc.
 * 
 * WvUDPStream can send and receive packets on a connectionless UDP socket.
 * See wvudp.h for details.
 */
#include "wvudp.h"


WvUDPStream::WvUDPStream(const WvIPPortAddr &_local, const WvIPPortAddr &_rem)
	: remaddr(_rem)
{
    fd = socket(PF_INET, SOCK_DGRAM, 0);
    if (fd < 0 || fcntl(fd, F_SETFD, 1))
    {
	seterr(errno);
	return;
    }
    
    struct sockaddr *sa = _local.sockaddr();
    if (bind(fd, sa, _local.sockaddr_len()))
    {
	delete sa;
	seterr(errno);
	return;
    }
    delete sa;
    
    if (_rem != WvIPAddr())
    {
	struct sockaddr *sa = _rem.sockaddr();
	if (connect(fd, sa, _rem.sockaddr_len()))
	{
	    delete sa;
	    seterr(errno);
	    return;
	}
	delete sa;
    }
}


WvUDPStream::~WvUDPStream()
{
}


const WvAddr *WvUDPStream::src() const
{
    return &remaddr;
}


size_t WvUDPStream::uread(void *buf, size_t count)
{
    if (!isok() || !buf || !count) return 0;
    
    struct sockaddr_in from;
    size_t fromlen = sizeof(from);
    int in = recvfrom(getfd(), buf, count, 0, (sockaddr *)&from, &fromlen);
    
    if (in >= 0)
	remaddr = WvIPPortAddr(&from);

    // errors in UDP are ignored
    return in < 0 ? 0 : in;
}


size_t WvUDPStream::uwrite(const void *buf, size_t count)
{
    if (!isok() || !buf || !count) return 0;
    
    // usually people ignore the return value of write(), so we make
    // a feeble attempt to continue even if interrupted.
    struct sockaddr *to = remaddr.sockaddr();
    size_t tolen = remaddr.sockaddr_len();
    int out;
    
    out = sendto(getfd(), buf, count, 0, to, tolen);
    free(to);
    
    // errors in UDP are ignored
    return out < 0 ? 0 : out;
}


