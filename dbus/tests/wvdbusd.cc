#include "wvdbusserver.h"
#include "wvstreamsdaemon.h"

class WvDBusDaemon : public WvStreamsDaemon
{
public:
    WvDBusDaemon() :
        WvStreamsDaemon("WvDBusDaemon", WVSTREAMS_RELEASE,
			wv::bind(&WvDBusDaemon::cb, this)),
        log("WvDBusDaemon", WvLog::Debug)
    {
	args.add_required_arg("MONIKER", true);
    }
    
    virtual ~WvDBusDaemon() {}
    
    void cb()
    { 
        log("WvDBusDaemon starting.\n");

	WvDBusServer *s = new WvDBusServer;
	WvStringList::Iter i(extra_args());
	for (i.rewind(); i.next(); )
	    s->listen(*i);
	add_die_stream(s, true, "DBus Server");
    }

private:
    WvLog log;
};


int main(int argc, char *argv[])
{
    return WvDBusDaemon().run(argc, argv);
}
