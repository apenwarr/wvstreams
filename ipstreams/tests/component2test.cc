#include "wvmoniker.h"
#include "wvistreamlist.h"
#include "wvstreamclone.h"
#include "wvlog.h"
#include "wvlogrcv.h"

#include <assert.h>


int main()
{
    WvLogConsole rcv(2, WvLog::Debug2);
    
    // get streams from a component
    IWvStream *url1 = wvcreate<IWvStream>("http://mai/");
    IWvStream *url2 = wvcreate<IWvStream>("https://mai/~apenwarr/");
    
    // add handy WvStreams-style functionality by cloning them
    WvStreamClone a(url1);
    WvStreamClone b(url2);

    // make them do something useful
    WvLog l1("log1", WvLog::Info);
    WvLog l2("log2", WvLog::Info);
    a.autoforward(l1);
    b.autoforward(l2);
    
    // create a list of them (in fact, we could use a WvStreamList here...)
    WvIStreamList l;
    l.append(&a, false);
    l.append(&b, false);
  
    while (a.isok() || b.isok())
    {
	if (l.select(-1))
	    l.callback();
    }
    
    if (a.geterr())
	wvcon->print("url1: %s\n", a.errstr());
    if (b.geterr())
	wvcon->print("url2: %s\n", b.errstr());
    
    return 0;
}
