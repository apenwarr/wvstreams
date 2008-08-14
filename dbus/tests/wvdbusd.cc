#include "wvdbusserver.h"
#include "wvstreamsdaemon.h"
#include "wvautoconf.h"
#include "wvx509mgr.h"
#include "wvsslstream.h"
#include "wvmoniker.h"
#include "uniconfroot.h"


static WvX509Mgr *cert = NULL;


class WvDBusDaemon : public WvStreamsDaemon
{
public:
    WvDBusDaemon() :
        WvStreamsDaemon("WvDBusDaemon", WVPACKAGE_VERSION,
			wv::bind(&WvDBusDaemon::cb, this)),
        log("WvDBusDaemon", WvLog::Debug), configfile("wvdbus.ini")
    {
	args.add_option('c', "config", "Specify path to configuration file",
			    "FILENAME", configfile);
	args.add_required_arg("MONIKER", true);
    }
    
    virtual ~WvDBusDaemon()
    {
	WVRELEASE(cert);
    }
    
    void cb()
    { 
        log("WvDBusDaemon starting.\n");
	conf.mount(WvString("ini:%s", configfile));

	if (!cert && conf["cert"].exists() && conf["privrsa"].exists())
	{
	    cert = new WvX509Mgr;
	    cert->decode(WvX509::CertPEM, *conf["cert"]);
	    cert->decode(WvRSAKey::RsaPEM, *conf["privrsa"]);

	    if (!cert->test())
	    {
		log("Certificate found in ini file, but failed to load!\n");
		WVRELEASE(cert);
	    }
	    else
		log("Certificate found in ini file, and loaded!\n");
	}

	WvDBusServer *s = new WvDBusServer;
	WvStringList::Iter i(extra_args());
	for (i.rewind(); i.next(); )
	    s->listen(*i);
	add_die_stream(s, true, "DBus Server");
    }

private:
    WvLog log;
    UniConfRoot conf;
    WvString configfile;
};


static IWvStream *dbus_serv_creator(WvStringParm s, IObject *obj)
{
    return new WvSSLStream(IWvStream::create(s, obj), cert, 0, true);
}

static WvMoniker<IWvStream> sreg("sslserv", dbus_serv_creator, true);


int main(int argc, char *argv[])
{
    return WvDBusDaemon().run(argc, argv);
}
