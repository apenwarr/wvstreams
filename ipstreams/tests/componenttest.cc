#include "wvmoniker.h"
#include "wvistreamlist.h"
#include "wvstreamclone.h"

#include <assert.h>


static void httpget(WvStream &s, void *userdata)
{
    static int countdown = 6;
    
    if (s.alarm_was_ticking)
    {
	if (!--countdown)
	{
	    wvcon->print("...sending to TCP stream.\n");
	    s.print("GET / HTTP/1.0\r\n\r\n");
	}
	else
	    wvcon->print("[%s]", countdown);
    }
    
    WvStream::autoforward_callback(s, (WvStream *)userdata);
	
    s.alarm(1000);
}


int main()
{
    IWvStream *con  = wvcon;
    
    // get streams from a component
    IWvStream *ssl  = wvcreate<IWvStream>("ssl:tcp:192.168.12.1:995");
    IWvStream *tcp2 = wvcreate<IWvStream>("tcp:192.168.12.1:80");
    
    // add handy WvStreams-style functionality by cloning them
    WvStreamClone a(con);
    a.disassociate_on_close = true;
    WvStreamClone b(ssl);
    WvStreamClone c(tcp2);

    // make them do something useful
    a.autoforward(b);
    b.autoforward(a);
    c.setcallback(httpget, &a);
    c.alarm(1*1000);
    
    // create a list of them (in fact, we could use a WvStreamList here...)
    WvIStreamList l;
    WvIStreamList::globallist.append(&a, false);
    l.append(&b, false);
    l.append(&c, false);
    
    while (a.isok() && b.isok() && c.isok())
    {
	if (l.select(-1))
	    l.callback();
    }
    
    if (b.geterr())
	wvcon->print("tcp1: %s\n", b.errstr());
    if (c.geterr())
	wvcon->print("tcp2: %s\n", c.errstr());
    
    return 0;
}
