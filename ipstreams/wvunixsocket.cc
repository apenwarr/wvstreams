/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * WvStream-based Unix domain socket connection class.  See wvunixsocket.h.
 */
#include "wvistreamlist.h"
#include "wvunixsocket.h"
#include "wvmoniker.h"

#if HAVE_ERRNO_H
# include <errno.h>
#endif
#include <stdio.h>
#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#if STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# if HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif
#if HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#if HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
#if HAVE_NETDB_H
# include <netdb.h>
#endif
#if HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif
#if HAVE_NETINET_IP_H
# if HAVE_NETINET_IN_SYSTM_H
#  include <netinet/in_systm.h>
# endif
# include <netinet/ip.h>
#endif
#if HAVE_NETINET_TCP_H
# include <netinet/tcp.h>
#endif

#include <fcntl.h>
#include <sys/un.h>

static IWvStream *creator(WvStringParm s, IObject *, void *)
{
    return new WvUnixConn(s);
}

static WvMoniker<IWvStream> reg("unix", creator);


WvUnixConn::WvUnixConn(int _fd, const WvUnixAddr &_addr)
    : WvFDStream(_fd), addr(_addr)
{
    // all is well and we're connected.
    set_nonblock(true);
    set_close_on_exec(true);
}


WvUnixConn::WvUnixConn(const WvUnixAddr &_addr)
    : addr(_addr)
{
    setfd(socket(PF_UNIX, SOCK_STREAM, 0));
    if (getfd() < 0)
    {
	seterr(errno);
	return;
    }
    
    sockaddr *sa = addr.sockaddr();
    if (connect(getfd(), sa, addr.sockaddr_len()) < 0)
    {
	seterr(errno);
	delete sa;
	return;
    }
    
    delete sa;
    
    // all is well and we're connected.
    set_nonblock(true);
    set_close_on_exec(true);
}


WvUnixConn::~WvUnixConn()
{
    // we don't want to delete the socket file here; that's a job for the
    // listener class.
    
    // close the socket
    close();
}


const WvUnixAddr *WvUnixConn::src() const
{
    return &addr;
}


WvUnixListener::WvUnixListener(const WvUnixAddr &_addr, int create_mode)
	: addr(_addr)
{
    mode_t oldmask;
    
    auto_list = NULL;
    auto_userdata = NULL;
    bound_okay = false;
    
    setfd(socket(PF_UNIX, SOCK_STREAM, 0));
    if (getfd() < 0 || fcntl(getfd(), F_SETFD, 1))
    {
	seterr(errno);
	return;
    }
    
    sockaddr *sa = addr.sockaddr();
    size_t salen = addr.sockaddr_len();
    
    if (connect(getfd(), sa, salen) == 0) // successful connect?!
	seterr(EADDRINUSE); // someone is using this already!
    else
    {
	// unfortunately we have to change the umask here to make the
	// create_mode work, because bind() doesn't take extra arguments
	// like open() does. However, we don't actually want to _cancel_
	// the effects of umask, only add to them; so the original umask is
	// or'ed into ~create_mode.
	oldmask = umask(0777); // really just reading the old umask here
	umask(oldmask | ((~create_mode) & 0777));
	
	::unlink(WvString(addr));
	
	if (bind(getfd(), sa, salen) || listen(getfd(), 50))
	    seterr(errno);
	else
	    bound_okay = true;

        umask(oldmask);
    }
    
    delete sa;
}


WvUnixListener::~WvUnixListener()
{
    close();
}


void WvUnixListener::close()
{
    // delete the socket _before_ closing it.  Unix will keep
    // existing connections around anyway (if any), but if it's idle, then
    // we never have an existing not-in-use socket inode.
    if (bound_okay)
    {
	WvString filename(addr);
	::unlink(filename);
    }
    
    WvFDStream::close();
}


WvUnixConn *WvUnixListener::accept()
{
    struct sockaddr_un saun;
    socklen_t len = sizeof(saun);
    int newfd;
    WvUnixConn *ret;

    newfd = ::accept(getfd(), (struct sockaddr *)&saun, &len);
    ret = new WvUnixConn(newfd, addr);
    return ret;
}


void WvUnixListener::auto_accept(WvIStreamList *list,
				 WvStreamCallback callfunc, void *userdata)
{
    auto_list = list;
    auto_callback = callfunc;
    auto_userdata = userdata;
    setcallback(accept_callback, this);
}


void WvUnixListener::accept_callback(WvStream &, void *userdata)
{
    WvUnixListener &l = *(WvUnixListener *)userdata;

    WvUnixConn *connection = l.accept();
    connection->setcallback(l.auto_callback, l.auto_userdata);
    l.auto_list->append(connection, true);
}


size_t WvUnixListener::uread(void *, size_t)
{
    return 0;
}


size_t WvUnixListener::uwrite(const void *, size_t)
{
    return 0;
}


const WvUnixAddr *WvUnixListener::src() const
{
    return &addr;
}

