#include "wvistreamlist.h"
#include "wvlog.h"
#include "pwvstream.h"
#include "wvstreamclone.h"
#include "wvlinkerhack.h"
#include <signal.h>

WV_LINK_TO(WvConStream);
WV_LINK_TO(WvTCPConn);


volatile bool want_to_die = false;

static void signalhandler(int sig)
{
    fprintf(stderr, "Caught signal %d.  Exiting...\n", sig);
    want_to_die = true;
    signal(sig, SIG_DFL);
}


static void bounce_to_list(IWvStream *in, WvIStreamList *list)
{
    char buf[4096];
    size_t len;
    
    len = in->read(buf, sizeof(buf));
    
    WvIStreamList::Iter i(*list);
    for (i.rewind(); i.next(); )
    {
	if (in != i.ptr())
	{
	    // you might think this assumes IWvStream has a buffer; but in
	    // fact, we already know that everything in the list is a
	    // WvStreamClone, and WvStreamClone *does* have an output
	    // buffer, so this is safe.
	    i->write(buf, len);
	}
    }
}


int main(int argc, char **argv)
{
    WvIStreamList list;
    WvLog log(argv[0], WvLog::Debug);
    
    signal(SIGTERM, signalhandler);
    signal(SIGINT, signalhandler);

    if (argc <= 1)
    {
	fprintf(stderr, "Usage: %s <stream1> [stream2 [stream3...]]\n",
		argv[0]);
	return 1;
    }
    
    for (int count = 1; count < argc; count++)
    {
	log("Creating stream: '%s'\n", argv[count]);
	PWvStream s(argv[count]);
	if (!s)
	{
	    fprintf(stderr, "Can't create stream %s: no moniker!\n",
		    argv[count]);
	    return 2;
	}
	
	if (!s->isok())
	{
	    fprintf(stderr, "Stream %s: %s\n",
		    argv[count], s->errstr().cstr());
	    return 3;
	}
	
	s->setcallback(wv::bind(bounce_to_list, s.get(), &list));
	list.append(s.addRef(), true, argv[count]);
    }
    
    while (!want_to_die && list.count() >= 2)
	list.runonce();
}
