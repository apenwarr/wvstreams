#include "wvautoconf.h"
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#ifdef HAVE_GETOPT_H
# include <getopt.h>
#endif

#ifndef _WIN32
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

#ifdef WITH_SLP
#include "wvslp.h"
#endif

#include "wvlogrcv.h"
#include "uniconfdaemon.h"
#include "uniclientconn.h"
#include "unisecuregen.h"
#include "unipermgen.h"
#include "wvx509mgr.h"
#include "uniconfroot.h"
#include "wvstrutils.h"
#include "wvfileutils.h"
#include "wvstreamsdaemon.h"

#ifdef WITH_SLP
#include "slp.h"
#endif

#ifdef _WIN32
#pragma comment(linker, "/include:?UniRegistryGenMoniker@@3V?$WvMoniker@VIUniConfGen@@@@A")
#pragma comment(linker, "/include:?UniPStoreGenMoniker@@3V?$WvMoniker@VIUniConfGen@@@@A")
#pragma comment(linker, "/include:?UniIniGenMoniker@@3V?$WvMoniker@VIUniConfGen@@@@A")
#endif

#define DEFAULT_CONFIG_FILE "ini:uniconf.ini"


static WvMap<WvString, IUniConfGen*> namedgens(42);


static IUniConfGen *creator(WvStringParm s)
{
    IUniConfGen* gen = namedgens[s];

    if (gen)
	gen->addRef();

    return gen;
}

WvMoniker<IUniConfGen> UniNamedMoniker("named", creator);


class UniConfd : public WvStreamsDaemon
{
    bool needauth;
    int port;
    int sslport;
    WvString unixport, permmon, unix_mode;
    time_t commit_interval;

    UniConfRoot cfg;
    bool first_time;
    IUniConfGen *permgen;
    
    bool namedgen_cb(WvStringParm option, void *)
    {
	WvString name(option);
	WvString moniker;
	char* ptr;

	ptr = strchr(name.edit(), '=');

	if (!ptr)
	    return false;

	*ptr = 0;
	moniker = ptr + 1;

	namedgens.add(name, wvcreate<IUniConfGen>(moniker), true);
	return true;
    }

    void commit_stream_cb(WvStream &s, void *)
    {
	cfg.commit();
	cfg.refresh();
	if (permgen)
	    permgen->refresh();
	    
        s.alarm(commit_interval * 1000);
    }
    
    void startup(WvStreamsDaemon &, void *)
    {
        if (first_time)
        {
            WvStringList::Iter i(_extra_args);
            for (i.rewind(); i.next(); )
            {
	        WvString path = *i, moniker;
	        char *cptr = strchr(path.edit(), '=');
	        if (!cptr)
	        {
	            moniker = path;
	            path = "/";
	        }
	        else
	        {
	            *cptr = 0;
	            moniker = cptr+1;
	        }
	        
	        log("Mounting '%s' on '%s': ", moniker, path);
	        IUniConfGen *gen = cfg[path].mount(moniker, false);
	        if (gen && gen->isok())
	            log("ok.\n");
	        else
	            log("FAILED!\n");
            }
            
            cfg.refresh();
        }

        permgen = !!permmon ? wvcreate<IUniConfGen>(permmon) : NULL;
        
        UniConfDaemon *daemon = new UniConfDaemon(cfg, needauth, permgen);
        add_die_stream(daemon, true, "uniconfd");

#ifndef _WIN32
        if (!!unixport)
        {
	    // FIXME: THIS IS NOT SAFE!
	    mkdirp(getdirname(unixport));
	    ::unlink(unixport);
	    if (!daemon->setupunixsocket(unixport))
	        exit(3);
	    if (!!unix_mode && unix_mode.num())
	    {
	        log("Setting mode on %s to: %s\n", unixport, unix_mode);
	        mode_t mode;
	        sscanf(unix_mode.edit(), "%o", &mode);
	        chmod(unixport, mode);
	    }
        }
#endif

        if (port && !daemon->setuptcpsocket(WvIPPortAddr("0.0.0.0", port)))
        {
	    die();
	    return;
	}

        if (sslport)
        {
            WvString dName = encode_hostname_as_DN(fqdomainname());
            WvX509Mgr *x509cert = new WvX509Mgr(dName, 1024);
            if (!x509cert->isok())
            {
                log(WvLog::Critical,
		    "Couldn't generate X509 certificate: SSL not available.\n");
	        die();
	        return;
            }
            else if (!daemon->setupsslsocket(
		        WvIPPortAddr("0.0.0.0", sslport), x509cert))
	    {
	        die();
	        return;
	    }
        }
    
#ifdef WITH_SLP
        WvSlp slp;

        if (first_time)
        {
            // Now that we're this far...
            
            // Register UniConf service with SLP 
            if (port)
	        slp.add_service("uniconf.niti", fqdomainname(), port);
        
            // Register UniConf SSL service with SLP 
            if (sslport)
	        slp.add_service("uniconfs.niti", fqdomainname(), sslport);
	}
#endif
    
        WvStream *commit_stream = new WvStream;
        commit_stream->setcallback(WvStreamCallback(this,
                &UniConfd::commit_stream_cb), NULL);
        commit_stream->alarm(commit_interval * 1000);
        add_die_stream(commit_stream, true, "commit");
        
        if (first_time)
            first_time = false;
    }
    
public:

    UniConfd() :
            WvStreamsDaemon("uniconfd", VERBOSE_PACKAGE_VERSION,
                WvStreamsDaemonCallback(this, &UniConfd::startup)),
            needauth(false),
            port(DEFAULT_UNICONF_DAEMON_TCP_PORT),
            sslport(DEFAULT_UNICONF_DAEMON_SSL_PORT),
            commit_interval(5*60),
            first_time(true),
            permgen(NULL)
    {
        args.add_option(0, "pid-file",
                "Specify the .pid file to use (only applies with --daemonize)", "filename",
                pid_file);
        args.add_set_bool_option('a', "need-auth",
                "Require authentication on incoming connections", needauth);
        args.add_option('A', "check-access",
                "Check all accesses against perms moniker", "moniker",
                permmon);
        args.add_option('p', "tcp",
                "Listen on given TCP port (default=4111; 0 to disable)", "port",
                port);
        args.add_option('s', "ssl",
                "Listen on given TCP/SSL port (default=4112; 0 to disable)", "port",
                sslport);
#ifndef _WIN32
        args.add_option('u', "unix",
                "Listen on given Unix socket filename (default=disabled)", "filename",
                unixport);
        args.add_option('m', "unix-mode",
                "Set the Unix socket to 'mode' (default to uniconfd users umask)", "mode",
                unix_mode);
	args.add_option('n', "named-gen",
		"creates a \"named\" moniker 'name' from 'moniker'",
		"name=moniker",
		WvArgs::ArgCallback(this, &UniConfd::namedgen_cb), NULL);
#endif
	args.add_optional_arg("MONIKERS", true);
	args.set_email("<" PACKAGE_BUGREPORT ">");
    }
    
    
};

int main(int argc, char **argv)
{
    UniConfd uniconfd;
   
    return uniconfd.run(argc, argv);
}
