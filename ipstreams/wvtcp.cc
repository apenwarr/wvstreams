/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997, 1998 Worldvisions Computer Technology, Inc.
 * 
 * WvStream-based TCP connection class.
 */
#include "wvstreamlist.h"
#include "wvtcp.h"
#include <errno.h>

WvStreamList WvTCPListener::all_listeners;


WvTCPConn::WvTCPConn(const WvIPPortAddr &_remaddr)
{
    remaddr = _remaddr;
    resolved = true;
    connected = false;
    do_connect();
}


WvTCPConn::WvTCPConn(int _fd, const WvIPPortAddr &_remaddr)
	: WvStream(_fd)
{
    remaddr = _remaddr;
    resolved = true;
    connected = true;
    
    if (fd < 0)
	seterr(errno);
}


WvTCPConn::WvTCPConn(const WvString &_hostname, __u16 _port)
	: hostname(_hostname)
{
    char *hnstr = hostname.edit(), *cptr;
    
    cptr = strchr(hnstr, ':');
    if (!cptr)
	cptr = strchr(hnstr, '\t');
    if (!cptr)
	cptr = strchr(hnstr, ' ');
    if (cptr)
    {
	*cptr++ = 0;
	remaddr.port = atoi(cptr);
    }
    
    resolved = connected = false;
    
    dns.findaddr(0, hostname, NULL);
    if (_port)
	remaddr.port = _port;
}


void WvTCPConn::do_connect()
{
    sockaddr *sa;

    fd = socket(PF_INET, SOCK_STREAM, 0);
    if (fd < 0
	|| fcntl(fd, F_SETFD, 1)
	|| fcntl(fd, F_SETFL, O_RDWR|O_NONBLOCK) < 0)
    {
	seterr(errno);
	return;
    }
    
    sa = remaddr.sockaddr();
    if (connect(fd, sa, remaddr.sockaddr_len()) < 0
	&& errno != EINPROGRESS)
    {
	seterr(errno);
	delete sa;
	return;
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


WvIPPortAddr WvTCPConn::localaddr()
{
    sockaddr_in sin;
    size_t sl = sizeof(sin);
    
    if (!isok() || getsockname(getfd(), (sockaddr *)&sin, &sl))
	return WvIPPortAddr();
    
    return WvIPPortAddr(&sin);
}


const WvAddr *WvTCPConn::src() const
{
    return &remaddr;
}


bool WvTCPConn::select_setup(SelectInfo &si)
{
    if (!resolved)
    {
	if (dns.select_setup(hostname, si))
	    check_resolver();
    }

    if (resolved && isok()) // name might be resolved now.
    {
	bool oldw = si.writable, retval;
	if (!isconnected()) si.writable = true;
	retval = WvStream::select_setup(si);
	si.writable = oldw;
	return retval;
    }
    else
	return false;
}
			  
bool WvTCPConn::test_set(SelectInfo &si)
{
    bool result = false;

    if (!resolved)
	check_resolver();
    else
    {
	result = WvStream::test_set(si);

	if (result && !connected)
	{
	    sockaddr *sa = remaddr.sockaddr();
	    int retval = connect(fd, sa, remaddr.sockaddr_len());
	    
	    if (!retval || (retval < 0 && errno == EISCONN))
		connected = result = true;
	    else if (retval < 0 && errno != EINPROGRESS)
	    {
		seterr(errno);
		result = true;
	    }
	    else
		result = false;
	    
	    delete sa;
	}
    }
    
    return result;
}


bool WvTCPConn::isok() const
{
    return !resolved || WvStream::isok();
}


WvTCPListener::WvTCPListener(const WvIPPortAddr &_listenport)
	: listenport(_listenport)
{
    listenport = _listenport;
    auto_list = NULL;
    auto_callback = NULL;
    auto_userdata = NULL;
    
    sockaddr *sa = listenport.sockaddr();
    
    int x = 1;

    fd = socket(PF_INET, SOCK_STREAM, 0);
    if (fd < 0
	|| setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &x, sizeof(x))
	|| fcntl(fd, F_SETFD, 1)
	|| bind(fd, sa, listenport.sockaddr_len())
	|| listen(fd, 5))
    {
	seterr(errno);
    }
    
    if (listenport.port == 0) // auto-select a port number
    {
	size_t namelen = listenport.sockaddr_len();
	
	if (getsockname(fd, sa, &namelen) != 0)
	    seterr(errno);
	else
	    listenport = WvIPPortAddr((sockaddr_in *)sa);
    }
    
    delete sa;
    
    all_listeners.append(this, false);
}


WvTCPListener::~WvTCPListener()
{
    close();
    all_listeners.unlink(this);
}


//#include <wvlog.h>
void WvTCPListener::close()
{
    WvStream::close();
/*    WvLog log("ZAP!");
    
    log("Closing TCP LISTENER at %s!!\n", listenport);
    abort();*/
}


WvTCPConn *WvTCPListener::accept()
{
    struct sockaddr_in sin;
    size_t len = sizeof(sin);
    int newfd;
    WvTCPConn *ret;

    newfd = ::accept(fd, (struct sockaddr *)&sin, &len);
    ret = new WvTCPConn(newfd, WvIPPortAddr(&sin));
    return ret;
}


void WvTCPListener::auto_accept(WvStreamList *list,
				Callback *callfunc, void *userdata)
{
    auto_list = list;
    auto_callback = callfunc;
    auto_userdata = userdata;
    setcallback(accept_callback, this);
}


void WvTCPListener::accept_callback(WvStream &, void *userdata)
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


const WvAddr *WvTCPListener::src() const
{
    return &listenport;
}


void WvTCPListener::close_all_listeners()
{
    WvStreamList::Iter i(all_listeners);
    for (i.rewind(); i.next(); )
	i().close();
}
