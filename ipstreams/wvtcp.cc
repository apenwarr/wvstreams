/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * WvStream-based TCP connection class.
 */
#include "wvtcp.h"
#include "wvistreamlist.h"
#include "wvmoniker.h"
#include "wvlinkerhack.h"
#include <fcntl.h>

#ifdef _WIN32
#define setsockopt(a,b,c,d,e) setsockopt(a,b,c, (const char*) d,e)
#define getsockopt(a,b,c,d,e) getsockopt(a,b,c,(char *)d, e) 
#undef errno
#define errno GetLastError()
#define EWOULDBLOCK WSAEWOULDBLOCK
#define EINPROGRESS WSAEINPROGRESS
#define EISCONN WSAEISCONN
#define EALREADY WSAEALREADY
#undef EINVAL
#define EINVAL WSAEINVAL
#define SOL_TCP IPPROTO_TCP
#define SOL_IP IPPROTO_IP
#define FORCE_NONZERO 1
#else
# if HAVE_STDLIB_H
#  include <stdlib.h>
# endif
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

#ifndef FORCE_NONZERO
#define FORCE_NONZERO 0
#endif

WV_LINK(WvTCPConn);


static IWvStream *creator(WvStringParm s, IObject *, void *)
{
    return new WvTCPConn(s);
}

static WvMoniker<IWvStream> reg("tcp", creator);


WvTCPConn::WvTCPConn(const WvIPPortAddr &_remaddr)
{
    remaddr = (_remaddr.is_zero() && FORCE_NONZERO)
	? WvIPPortAddr("127.0.0.1", _remaddr.port) : _remaddr;
    resolved = true;
    connected = false;
    incoming = false;
    
    do_connect();
}


WvTCPConn::WvTCPConn(int _fd, const WvIPPortAddr &_remaddr)
    : WvFDStream(_fd)
{
    remaddr = (_remaddr.is_zero() && FORCE_NONZERO)
	? WvIPPortAddr("127.0.0.1", _remaddr.port) : _remaddr;
    resolved = true;
    connected = true;
    incoming = true;
    nice_tcpopts();
}


WvTCPConn::WvTCPConn(WvStringParm _hostname, uint16_t _port)
    : hostname(_hostname)
{
    struct servent* serv;
    char *hnstr = hostname.edit(), *cptr;
    
    cptr = strchr(hnstr, ':');
    if (!cptr)
	cptr = strchr(hnstr, '\t');
    if (!cptr)
	cptr = strchr(hnstr, ' ');
    if (cptr)
    {
	*cptr++ = 0;
	serv = getservbyname(cptr, NULL);
	remaddr.port = serv ? ntohs(serv->s_port) : atoi(cptr);
    }
    
    if (_port)
	remaddr.port = _port;
    
    resolved = connected = false;
    incoming = false;
    
    WvIPAddr x(hostname);
    if (x != WvIPAddr())
    {
	remaddr = WvIPPortAddr(x, remaddr.port);
	resolved = true;
	do_connect();
    }
    else
	dns.findaddr(0, hostname, NULL);
}


WvTCPConn::~WvTCPConn()
{
    // nothing to do
}


// Set a few "nice" options on our socket... (read/write, non-blocking, 
// keepalive)
void WvTCPConn::nice_tcpopts()
{
    set_close_on_exec(true);
    set_nonblock(true);
    
    int value = 1;
    setsockopt(getfd(), SOL_SOCKET, SO_KEEPALIVE, &value, sizeof(value));
}


void WvTCPConn::low_delay()
{
    int value;
    
    value = 1;
    setsockopt(getfd(), SOL_TCP, TCP_NODELAY, &value, sizeof(value));
    
#ifndef _WIN32
    value = IPTOS_LOWDELAY;
    setsockopt(getfd(), SOL_IP, IP_TOS, &value, sizeof(value));
#endif
}


void WvTCPConn::debug_mode()
{
    int value = 0;
    setsockopt(getfd(), SOL_SOCKET, SO_KEEPALIVE, &value, sizeof(value));
}

void WvTCPConn::do_connect()
{
    if (getfd() < 0)
    {
	int rwfd = socket(PF_INET, SOCK_STREAM, 0);
	if (rwfd < 0)
	{
	    seterr(errno);
	    return;
	}
	setfd(rwfd);
	
	nice_tcpopts();
    }
    
    sockaddr *sa = remaddr.sockaddr();
    int ret = connect(getfd(), sa, remaddr.sockaddr_len()), err = errno;
    assert(ret <= 0);
    
    if (ret == 0 || (ret < 0 && err == EISCONN))
	connected = true;
    else if (ret < 0
	     && err != EINPROGRESS
	     && err != EWOULDBLOCK
	     && err != EAGAIN
	     && err != EALREADY
	     && err != EINVAL /* apparently winsock 1.1 might do this */)
    {
	connected = true; // "connection phase" is ended, anyway
	seterr(err);
    }
    delete sa;
}


void WvTCPConn::check_resolver()
{
    const WvIPAddr *ipr;
    int dnsres = dns.findaddr(0, hostname, &ipr);
    
    if (dnsres == 0)
    {
	// error resolving!
	resolved = true;
	seterr(WvString("Unknown host \"%s\"", hostname));
    }
    else if (dnsres > 0)
    {
	remaddr = WvIPPortAddr(*ipr, remaddr.port);
	resolved = true;
	do_connect();
    }
}

#ifndef SO_ORIGINAL_DST
# define SO_ORIGINAL_DST 80
#endif

WvIPPortAddr WvTCPConn::localaddr()
{
    sockaddr_in sin;
    socklen_t sl = sizeof(sin);
    
    if (!isok())
	return WvIPPortAddr();
    
    if (
#ifndef _WIN32
        // getsockopt() with SO_ORIGINAL_DST is for transproxy of incoming
        // connections.  For outgoing (and for windows) use just use good
        // old getsockname().
	(!incoming || getsockopt(getfd(), SOL_IP,
                                 SO_ORIGINAL_DST, (char*)&sin, &sl) < 0) &&
#endif
	getsockname(getfd(), (sockaddr *)&sin, &sl))
    {
	return WvIPPortAddr();
    }
    
    return WvIPPortAddr(&sin);
}


const WvIPPortAddr *WvTCPConn::src() const
{
    return &remaddr;
}


bool WvTCPConn::pre_select(SelectInfo &si)
{
    if (!resolved)
    {
	if (dns.pre_select(hostname, si))
	{
	    check_resolver();
	    if (!isok())
		return true; // oops, failed to resolve the name!
	}
    }

    if (resolved && isok()) // name might be resolved now.
    {
	bool oldw = si.wants.writable, retval;
	if (!isconnected()) {
	    si.wants.writable = true; 
#ifdef _WIN32
	    // WINSOCK INSANITY ALERT!
	    // 
	    // In Unix, you detect the success OR failure of a non-blocking
	    // connect() by select()ing with the socket in the write set.
	    // HOWEVER, in Windows, you detect the success of connect() by
	    // select()ing with the socket in the write set, and the
	    // failure of connect() by select()ing with the socket in the
	    // exception set!
	    si.wants.isexception = true;
#endif
	}
	retval = WvFDStream::pre_select(si);
	si.wants.writable = oldw;
	return retval;
    }
    else
	return false;
}
			  

bool WvTCPConn::post_select(SelectInfo &si)
{
    bool result = false;

    if (!resolved)
	check_resolver();
    else
    {
	result = WvFDStream::post_select(si);
	if (result && !connected)
	{
	    // the manual for connect() says just re-calling connect() later
	    // will return either EISCONN or the error code from the previous
	    // failed connection attempt.  However, in *some* OSes (like
	    // Windows, at least) a failed connection attempt resets the 
	    // socket back to "connectable" state, so every connect() call
	    // will just restart the background connecting process and we'll
	    // never get a result out.  Thus, we *first* check SO_ERROR.  If
	    // that returns no error, then maybe the socket is connected, or
	    // maybe they just didn't feel like giving us our error yet.
	    // Only then, call connect() to look for EISCONN or another error.
	    int conn_res = -1;
	    socklen_t res_size = sizeof(conn_res);
	    if (getsockopt(getfd(), SOL_SOCKET, SO_ERROR,
			   &conn_res, &res_size))
	    {
		// getsockopt failed
		seterr(errno);
		connected = true; // not in connecting phase anymore
	    }
	    else if (conn_res != 0)
	    {
		// connect failed
		seterr(conn_res);
		connected = true; // not in connecting phase anymore
	    }
	    else
	    {
		// connect succeeded!  Double check by re-calling connect().
		do_connect();
	    }
	}
    }
    
    return result;
}


bool WvTCPConn::isok() const
{
    return !resolved || WvFDStream::isok();
}


size_t WvTCPConn::uwrite(const void *buf, size_t count)
{
    if (connected)
	return WvFDStream::uwrite(buf, count);
    else
	return 0; // can't write yet; let them enqueue it instead
}




WvTCPListener::WvTCPListener(const WvIPPortAddr &_listenport)
	: listenport(_listenport)
{
    listenport = _listenport;
    auto_list = NULL;
    auto_userdata = NULL;
    
    sockaddr *sa = listenport.sockaddr();
    
    int x = 1;

    setfd(socket(PF_INET, SOCK_STREAM, 0));
    set_close_on_exec(true);
    set_nonblock(true);
    if (getfd() < 0
	|| setsockopt(getfd(), SOL_SOCKET, SO_REUSEADDR, &x, sizeof(x))
	|| bind(getfd(), sa, listenport.sockaddr_len())
	|| listen(getfd(), 5))
    {
	seterr(errno);
    }
    
    if (listenport.port == 0) // auto-select a port number
    {
	socklen_t namelen = listenport.sockaddr_len();
	
	if (getsockname(getfd(), sa, &namelen) != 0)
	    seterr(errno);
	else
	    listenport = WvIPPortAddr((sockaddr_in *)sa);
    }
    
    delete sa;
}


WvTCPListener::~WvTCPListener()
{
    close();
}


//#include <wvlog.h>
void WvTCPListener::close()
{
    WvFDStream::close();
/*    WvLog log("ZAP!");
    
    log("Closing TCP LISTENER at %s!!\n", listenport);
    abort();*/
}


WvTCPConn *WvTCPListener::accept()
{
    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    int newfd;
    WvTCPConn *ret;

    newfd = ::accept(getfd(), (struct sockaddr *)&sin, &len);
    ret = new WvTCPConn(newfd, WvIPPortAddr(&sin));
    return ret;
}


void WvTCPListener::auto_accept(WvIStreamList *list,
				WvStreamCallback callfunc, void *userdata)
{
    auto_list = list;
    auto_callback = callfunc;
    auto_userdata = userdata;
    setcallback(accept_callback, this);
}

void WvTCPListener::auto_accept(WvStreamCallback callfunc, void *userdata)
{
    auto_callback = callfunc;
    auto_userdata = userdata;
    setcallback(accept_global_callback, this);
}


void WvTCPListener::accept_global_callback(WvStream &s, void *userdata)
{
    WvTCPListener &l = *(WvTCPListener *)userdata; 
    WvTCPConn *connection = l.accept();
    connection->setcallback(l.auto_callback, l.auto_userdata);
    WvIStreamList::globallist.append(connection, true);
}


void WvTCPListener::accept_callback(WvStream &s, void *userdata)
{
    WvTCPListener &l = *(WvTCPListener *)userdata;

    WvTCPConn *connection = l.accept();
    connection->setcallback(l.auto_callback, l.auto_userdata);
    l.auto_list->append(connection, true);
}


size_t WvTCPListener::uread(void *, size_t)
{
    return 0;
}


size_t WvTCPListener::uwrite(const void *, size_t)
{
    return 0;
}


const WvIPPortAddr *WvTCPListener::src() const
{
    return &listenport;
}

