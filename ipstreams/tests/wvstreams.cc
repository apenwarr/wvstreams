#include "wvistreamlist.h"
#include "wvlog.h"
#include "wvmoniker.h"
#include "wvstreamclone.h"


static void bounce_to_list(WvStream &s, void *userdata)
{
    WvIStreamList *list = (WvIStreamList *)userdata;
    char buf[1024];
    size_t len;
    
    len = s.read(buf, sizeof(buf));
    if (!len) return;
    
    WvIStreamList::Iter i(*list);
    for (i.rewind(); i.next(); )
    {
	if (&s != i.ptr())
	{
	    // you might think this assumes IWvStream has a buffer; but in
	    // fact, we already know that everything in the list is a
	    // WvStreamClone, and WvStreamClone *does* have an output
	    // buffer, so this is safe.
	    // 
	    // but FIXME: the iswritable() check breaks everything, but is
	    // current necessary for 'stdin' to not close itself when
	    // written to.
	    if (i->iswritable())
		i->write(buf, len);
	}
    }
}


int main(int argc, char **argv)
{
    WvIStreamList list;
    WvLog log(argv[0], WvLog::Debug);
    
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
    
    while (list.count() >= 2)
	list.runonce();
}
