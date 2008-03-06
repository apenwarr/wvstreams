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


static void died(WvLog &log, WvStringParm name, IWvStream *s)
{
    if (s->geterr())
	log("%s: %s\n", name, s->errstr());
}


static void add(WvLog &log, WvIStreamList &list, const char *_mon)
{
    WvString mon(_mon);
    if (mon == "-")
	mon = "stdio";
    log("Creating stream: '%s'\n", mon);
    PWvStream s(mon);
    if (!s->isok())
	died(log, _mon, s.addRef());
    else
    {
	s->setcallback(wv::bind(bounce_to_list, s.get(), &list));
	s->setclosecallback(wv::bind(died, log, _mon, s.addRef()));
    }
    list.append(s.addRef(), true, _mon);
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
    
    if (argc == 2) // talking to just one stream means send it to stdio
	add(log, list, "-");
    
    for (int count = 1; count < argc; count++)
	add(log, list, argv[count]);
    
    while (!want_to_die && list.count() >= 2)
	list.runonce();
}
