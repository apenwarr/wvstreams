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
	args.add_required_arg("MONIKER");
    }
    
    virtual ~WvDBusDaemon() {}
    
    void cb()
    { 
        log("WvDBusDaemon starting..\n");

        WvString moniker = _extra_args.popstr();
        
        add_die_stream(new WvDBusServer(moniker), true, "DBus Server");
    }

private:
    WvLog log;
};


int main(int argc, char *argv[])
{
    return WvDBusDaemon().run(argc, argv);
}
