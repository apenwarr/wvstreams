/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * UDP test program.  I don't think this works...
 */
#include "wvstreamlist.h"
#include "wvlog.h"
#include "wvipraw.h"
#include "wvhashtable.h"
#include <time.h>

DeclareWvList(WvInPlaceBuf);

class WvIPRawListener;

class WvIPRawConn : public WvStream
{
    friend class WvIPRawListener;
    
public:
    WvIPRawConn(WvIPRawListener *_parent, const WvIPAddr &_remaddr);

    WvIPAddr remaddr;
    
protected:
    WvIPRawListener *parent;
    time_t last_receive;
    WvInPlaceBufList buflist;
    
    virtual bool isok() const;
    virtual size_t uread(void *buf, size_t size);
    virtual size_t uwrite(const void *buf, size_t size);
    
    void push(const void *buf, size_t size);
};


DeclareWvDict(WvIPRawConn, WvIPAddr, remaddr);


class WvIPRawListener : public WvIPRawStream
{
    friend class WvIPRawConn;
    WvIPRawConnDict connlist;
    
public:
    WvIPRawListener(const WvIPAddr &_localaddr);
    
    virtual void execute();
};



WvIPRawConn::WvIPRawConn(WvIPRawListener *_parent, const WvIPAddr &_remaddr)
    : remaddr(_remaddr)
{
    parent = _parent;
    time(&last_receive);
}


bool WvIPRawConn::isok() const
{
    return parent->connlist[remaddr] == this;
}


size_t WvIPRawConn::uread(void *buf, size_t size)
{
    if (!buflist.count()) return 0;

    WvInPlaceBufList::Iter i(buflist);
    i.rewind(); i.next();
    WvInPlaceBuf &b = *i;
    if (b.used() < size)
	size = b.used();
    memcpy(buf, b.get(size), size);
    i.unlink();
    
    return size;
}


size_t WvIPRawConn::uwrite(const void *buf, size_t size)
{
    parent->setdest(remaddr);
    return parent->write(buf, size);
}


void WvIPRawConn::push(const void *buf, size_t size)
{
    time(&last_receive);
    
    WvInPlaceBuf *b = new WvInPlaceBuf(size);
    b->put(buf, size);
    buflist.append(b, true);
}


WvIPRawListener::WvIPRawListener(const WvIPAddr &_localaddr)
    : WvIPRawStream(_localaddr, WvIPAddr()), connlist(7)
{
}


void WvIPRawListener::execute()
{
    WvIPRawStream::execute();
    
    unsigned char buf[2048]; // larger than an expected UDP packet
    size_t len;
    
    while (select(0))
    {
	len = read(buf, sizeof(buf));
	if (!len) continue;
	
	WvIPRawConn *conn = connlist[*(WvIPAddr *)src()];
	if (!conn)
	{
	    conn = new WvIPRawConn(this, *(WvIPAddr *)src());
	    connlist.add(conn, true);
	}
	
	conn->push(buf, len);
    }
}


int main(int argc, char **argv)
{
    WvLog err("ip2test", WvLog::Error);
    WvIPAddr localaddr(argc > 1 ? argv[1] : "0.0.0.0");
    WvIPRawListener sock(localaddr);
    
    err(WvLog::Info, "Local address is %s.\n", localaddr);
    
    wvcon->autoforward(sock);
    sock.autoforward(err);
    
    WvStreamList l;
    l.add_after(l.tail, wvcon, false);
    l.add_after(l.tail, &sock, false);
#if 0    // not done yet
    while (wvcon->isok() && sock.isok())
    {
	sock.setdest(remaddr);
	if (l.select(1000))
	{
	    if (wvcon->select(0))
		wvcon->callback();
	    else if (sock.select(0))
	    {
		sock.callback();
		err(WvLog::Info, "    (remote: %s)\n", *sock.src());
	    }
	}
    }
#endif
    if (!wvcon->isok() && wvcon->geterr())
	err("stdin: %s\n", strerror(wvcon->geterr()));
    else if (!sock.isok() && sock.geterr())
	err("socket: %s\n", strerror(sock.geterr()));
    
    return 0;
}
