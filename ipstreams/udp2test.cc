/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1998, 1999 Worldvisions Computer Technology, Inc.
 */
#include "wvstreamlist.h"
#include "wvlog.h"
#include "wvudp.h"
#include "wvhashtable.h"
#include <time.h>

class WvUDPListener;

class WvUDPConn : public WvStream
{
    friend WvUDPListener;
    
public:
    WvUDPConn(WvUDPListener *_parent, const WvIPPortAddr &_remaddr);

    WvIPPortAddr remaddr;
    
protected:
    WvUDPListener *parent;
    time_t last_receive;
    WvMiniBufferList buflist;
    
    virtual bool isok() const;
    virtual size_t uread(void *buf, size_t size);
    virtual size_t uwrite(const void *buf, size_t size);
    
    void push(const void *buf, size_t size);
};


DeclareWvDict(WvUDPConn, WvIPPortAddr, remaddr);


class WvUDPListener : public WvUDPStream
{
    friend WvUDPConn;
    WvUDPConnDict connlist;
    
public:
    WvUDPListener(const WvIPPortAddr &_localaddr);
    
    virtual void execute();
};



WvUDPConn::WvUDPConn(WvUDPListener *_parent, const WvIPPortAddr &_remaddr)
			: remaddr(_remaddr)
{
    parent = _parent;
    time(&last_receive);
}


bool WvUDPConn::isok() const
{
    return parent->connlist[remaddr] == this;
}


size_t WvUDPConn::uread(void *buf, size_t size)
{
    if (!buflist.count()) return 0;

    WvMiniBufferList::Iter i(buflist);
    i.rewind(); i.next();
    WvMiniBuffer &b = i;
    if (b.used() < size)
	size = b.used();
    memcpy(buf, b.get(size), size);
    i.unlink();
    
    return size;
}


size_t WvUDPConn::uwrite(const void *buf, size_t size)
{
    parent->setdest(remaddr);
    return parent->write(buf, size);
}


void WvUDPConn::push(const void *buf, size_t size)
{
    time(&last_receive);
    
    WvMiniBuffer *b = new WvMiniBuffer(size);
    b->put(buf, size);
    buflist.append(b, true);
}


WvUDPListener::WvUDPListener(const WvIPPortAddr &_localaddr)
	: WvUDPStream(_localaddr, WvIPPortAddr()), connlist(7)
{
}


void WvUDPListener::execute()
{
    unsigned char buf[2048]; // larger than an expected UDP packet
    size_t len;
    
    while (select(0))
    {
	len = read(buf, sizeof(buf));
	if (!len) continue;
	
	WvUDPConn *conn = connlist[*(WvIPPortAddr *)src()];
	if (!conn)
	{
	    conn = new WvUDPConn(this, *(WvIPPortAddr *)src());
	    connlist.add(conn, true);
	}
	
	conn->push(buf, len);
    }
}


int main(int argc, char **argv)
{
    WvLog err("udptest", WvLog::Error);
    WvIPPortAddr localaddr(argc > 1 ? argv[1] : "0.0.0.0:2222");
    WvUDPListener sock(localaddr);
    
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
