/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * WvStream-based Unix domain socket connection class.  See wvunixsocket.h.
 */
#include "wvistreamlist.h"
#include "wvunixlistener.h"
#include "wvunixsocket.h"
#include "wvstringmask.h"
#include "wvmoniker.h"
#include "wvlinkerhack.h"

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

WV_LINK(WvUnixConn);
WV_LINK(WvUnixListener);

static IWvStream *creator(WvStringParm s, IObject*)
{
    return new WvUnixConn(s);
}

static WvMoniker<IWvStream> reg("unix", creator);


static IWvListener *listener(WvStringParm s, IObject *)
{
    WvConstStringBuffer b(s);
    WvString path = wvtcl_getword(b);
    WvString wrapper = b.getstr();
    IWvListener *l = new WvUnixListener(path, 0777);
    if (l && !!wrapper)
	l->addwrap(wv::bind(&IWvStream::create, wrapper, _1));
    return l;
}

static IWvListener *modelistener(WvStringParm s, IObject *)
{
    WvConstStringBuffer b(s);
    
    // strtoul knows how to interpret octal if it starts with '0'
    int mode = strtoul(wvtcl_getword(b, WvStringMask(":")), NULL, 0);
    if (b.peekch() == ':')
	b.get(1);
    WvString path = wvtcl_getword(b);
    WvString wrapper = b.getstr();
    IWvListener *l = new WvUnixListener(path, mode);
    if (l && !!wrapper)
	l->addwrap(wv::bind(&IWvStream::create, wrapper, _1));
    return l;
}

static WvMoniker<IWvListener> lreg("unix", listener);
static WvMoniker<IWvListener> lmodereg("unixmode", modelistener);


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
    
    // Make the socket non-blocking and close-on-exec.
    fcntl(getfd(), F_SETFD, FD_CLOEXEC);
    fcntl(getfd(), F_SETFL, O_RDWR|O_NONBLOCK);
    
    sockaddr *sa = addr.sockaddr();
    if (connect(getfd(), sa, addr.sockaddr_len()) < 0)
	seterr(errno);
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
	: WvListener(new WvFdStream(socket(PF_UNIX, SOCK_STREAM, 0))),
          addr(_addr)
{
    WvFdStream *fds = (WvFdStream *)cloned;
    
    mode_t oldmask;
    bound_okay = false;
    
    if (getfd() < 0)
    {
	// error inherited from substream
	return;
    }
    
    fds->set_close_on_exec(true);
    fds->set_nonblock(true);

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
	// or'ed into ~create_mode.  This way it acts like open().
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
    
    WvListener::close();
}


IWvStream *WvUnixListener::accept()
{
    struct sockaddr_un saun;
    socklen_t len = sizeof(saun);
    
    if (!isok()) return NULL;
    
    int newfd = ::accept(getfd(), (struct sockaddr *)&saun, &len);
    if (newfd >= 0)
	return wrap(new WvUnixConn(newfd, addr));
    else if (errno == EAGAIN || errno == EINTR)
	return NULL; // this listener is doing weird stuff
    else
    {
	seterr(errno);
	return NULL;
    }
}


void WvUnixListener::accept_callback(WvIStreamList *list,
				    wv::function<void(IWvStream*)> cb,
				    IWvStream *_conn)
{
    WvStreamClone *conn = new WvStreamClone(_conn);
    conn->setcallback(wv::bind(cb, conn));
    list->append(conn, true, "WvUnixConn");
}


const WvUnixAddr *WvUnixListener::src() const
{
    return &addr;
}

