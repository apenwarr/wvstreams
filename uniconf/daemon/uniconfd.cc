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
#include "uniconfroot.h"
#include "wvstrutils.h"
#include "wvfileutils.h"
#include "wvstreamsdaemon.h"

#ifdef WITH_SLP
#include "slp.h"
#endif

#include <map>

using std::map;
using wv::shared_ptr;


#ifdef _WIN32
#pragma comment(linker, "/include:?UniRegistryGenMoniker@@3V?$WvMoniker@VIUniConfGen@@@@A")
#pragma comment(linker, "/include:?UniPStoreGenMoniker@@3V?$WvMoniker@VIUniConfGen@@@@A")
#pragma comment(linker, "/include:?UniIniGenMoniker@@3V?$WvMoniker@VIUniConfGen@@@@A")
#endif

#define DEFAULT_CONFIG_FILE "ini:uniconf.ini"


static map<WvString, shared_ptr<IUniConfGen> > namedgens;


static IUniConfGen *creator(WvStringParm s, IObject*)
{
    map<WvString, shared_ptr<IUniConfGen> >::iterator it = namedgens.find(s);
    shared_ptr<IUniConfGen> gen;

    if (it != namedgens.end())
	gen = it->second;

    if (gen)
	gen->addRef();

    return gen.get();
}

WvMoniker<IUniConfGen> UniNamedMoniker("named", creator);


class UniConfd : public WvStreamsDaemon
{
    bool needauth;
    WvString permmon;
    WvStringList lmonikers;
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

	namedgens[name] = shared_ptr<IUniConfGen>(
	    wvcreate<IUniConfGen>(moniker),
	    wv::bind(&IUniConfGen::release, _1));

	return true;
    }

    void commit_stream_cb(WvStream *s)
    {
	cfg.commit();
	cfg.refresh();
	if (permgen)
	    permgen->refresh();
	    
        s->alarm(commit_interval * 1000);
    }
    
    void startup()
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
	
	if (lmonikers.isempty())
	{
	    log(WvLog::Critical, "Can't start: no listeners given!\n");
	    die(7);
	    return;
	}

	WvStringList::Iter i(lmonikers);
	for (i.rewind(); i.next(); )
	    daemon->listen(*i);

        WvStream *commit_stream = new WvStream;
        commit_stream->setcallback(wv::bind(&UniConfd::commit_stream_cb, this,
					    commit_stream));
        commit_stream->alarm(commit_interval * 1000);
        add_die_stream(commit_stream, true, "commit");
        
        if (first_time)
            first_time = false;
    }
    
public:

    UniConfd():
	WvStreamsDaemon("uniconfd", VERBOSE_WVPACKAGE_VERSION,
			wv::bind(&UniConfd::startup, this)),
	needauth(false),
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
	args.add_option('l', "listen",
		"Listen on the given socket (eg. tcp:4111, ssl:tcp:4112)",
		"lmoniker", lmonikers);
	args.add_option('n', "named-gen",
			"creates a \"named\" moniker 'name' from 'moniker'",
			"name=moniker",
			wv::bind(&UniConfd::namedgen_cb, this, _1, _2), NULL);
	args.add_optional_arg("MONIKERS", true);
	args.set_email("<" WVPACKAGE_BUGREPORT ">");
    }
    
    
};

int main(int argc, char **argv)
{
    UniConfd uniconfd;
   
    return uniconfd.run(argc, argv);
}
