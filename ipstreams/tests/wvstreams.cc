#include "wvistreamlist.h"
#include "wvlog.h"
#include "wvmoniker.h"
#include "wvstreamclone.h"
#include <signal.h>

volatile bool want_to_die = false;

static void signalhandler(int sig)
{
    fprintf(stderr, "Caught signal %d.  Exiting...\n", sig);
    want_to_die = true;
    signal(sig, SIG_DFL);
}


static void bounce_to_list(WvStream &s, void *userdata)
{
    WvIStreamList *list = (WvIStreamList *)userdata;
    char buf[4096];
    size_t len;
    
    for (int i = 0; i < 1000; i++)
    {
	len = s.read(buf, sizeof(buf));
	if (!len) break;
	
	WvIStreamList::Iter i(*list);
	for (i.rewind(); i.next(); )
	{
	    if (&s != i.ptr())
	    {
		// you might think this assumes IWvStream has a buffer; but in
		// fact, we already know that everything in the list is a
		// WvStreamClone, and WvStreamClone *does* have an output
		// buffer, so this is safe.
		i->write(buf, len);
	    }
	}
    }
}


int main(int argc, char **argv)
{
    WvIStreamList list;
    WvLog log(argv[0], WvLog::Debug);
    
    signal(SIGTERM, signalhandler);
    signal(SIGINT, signalhandler);
    signal(SIGHUP, signalhandler);

    if (argc <= 1)
    {
	fprintf(stderr, "Usage: %s <stream1> [stream2 [stream3...]]\n",
		argv[0]);
	return 1;
    }
    
    for (int count = 1; count < argc; count++)
    {
	log("Creating stream: '%s'\n", argv[count]);
	IWvStream *s = wvcreate<IWvStream>(argv[count]);
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
	
	WvStream *s2 = new WvStreamClone(s);
	
	s2->setcallback(bounce_to_list, &list);
	list.append(s2, true, argv[count]);
    }
    
    while (!want_to_die && list.count() >= 2)
	list.runonce();
}
