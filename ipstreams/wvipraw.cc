/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * WvIPRawStream can send and receive packets on a connectionless IP socket.
 * See wvipraw.h for details.
 */
#include "wvipraw.h"
#include <sys/socket.h>
#include <fcntl.h>

#ifdef ISDARWIN
# define socklen_t int
#endif

WvIPRawStream::WvIPRawStream(const WvIPAddr &_local,
			     const WvIPAddr &_rem,
			     int ip_protocol) :
    localaddr(_local), remaddr(_rem)
{
    int x = 1;
    setfd(socket(PF_INET, SOCK_RAW, ip_protocol));
    if (getfd() < 0 
        || setsockopt(getfd(), SOL_SOCKET, SO_REUSEADDR, &x, sizeof(x)) < 0)
    {
	seterr(errno);
	return;
    }
    
    set_close_on_exec(true);
    set_nonblock(true);

    struct sockaddr *sa = _local.sockaddr();
    if (bind(getfd(), sa, _local.sockaddr_len()))
    {
	delete sa;
	seterr(errno);
	return;
    }
    delete sa;
    
    struct sockaddr_in nsa;
    socklen_t nsalen = sizeof(nsa);
    if (getsockname(getfd(), (sockaddr *)&nsa, &nsalen) < 0)
    {
	seterr(errno);
	return;
    }
    localaddr = WvIPAddr((sockaddr*)&nsa);
    
    if (WvIPAddr(_rem) != WvIPAddr())
    {
	struct sockaddr *sa = _rem.sockaddr();
	if (connect(getfd(), sa, _rem.sockaddr_len()))
	{
	    delete sa;
	    seterr(errno);
	    return;
	}
	delete sa;
    }
}


WvIPRawStream::~WvIPRawStream()
{
}


const WvAddr *WvIPRawStream::src() const
{
    return &remaddr;
}


const WvAddr *WvIPRawStream::local() const
{
    return &localaddr;
}


size_t WvIPRawStream::uread(void *buf, size_t count)
{
    if (!isok() || !buf || !count) return 0;
    
    struct sockaddr_in from;
    socklen_t fromlen = sizeof(from);
    int in = recvfrom(getfd(), buf, count, 0, (sockaddr *)&from, &fromlen);
    
    if (in >= 0)
	remaddr = WvIPAddr((sockaddr *)&from);

    // errors in IP are ignored
    return in < 0 ? 0 : in;
}


size_t WvIPRawStream::uwrite(const void *buf, size_t count)
{
    if (!isok() || !buf || !count) return 0;
    
    struct sockaddr *to = remaddr.sockaddr();
    size_t tolen = remaddr.sockaddr_len();
    int out;
    
    out = sendto(getfd(), buf, count, 0, to, tolen);
    
    if (out < 0 && errno == EACCES) // permission denied
	seterr(EACCES);
    
    free(to);
    
    // errors in UDP are ignored
    // pretend that the write always succeeds even if the kernel
    // complains since we don't want datagrams backing up in the
    // buffer and forming merged datagrams as a result
    return out < 0 ? 0 : out;
}


void WvIPRawStream::enable_broadcasts()
{
    int value = 1;
    
    if (!isok()) return;
    
    setsockopt(getfd(), SOL_SOCKET, SO_BROADCAST, &value, sizeof(value));
}
