/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * DNS name resolver with support for background lookups.
 */
#include "wvresolver.h"
#include "wvloopback.h"
#include "wvaddr.h"
#include "wvtcp.h"
#include <sys/types.h>
#include <signal.h>
#include <time.h>

#ifdef _WIN32
#define WVRESOLVER_SKIP_FORK
typedef int pid_t;
#define kill(a,b)
#define waitpid(a,b,c) (0)
#define alarm(a)
#else
#include "wvautoconf.h"
#include "wvfork.h"
#include <netdb.h>
#include <sys/wait.h>
#endif

class WvResolverHost
{
public:
    WvString name;
    WvIPAddr *addr;
    WvIPAddrList addrlist;
    bool done, negative;
    pid_t pid;
    WvLoopback *loop;
    time_t last_tried;

    WvResolverHost(WvStringParm _name) : name(_name)
        { init(); addr = NULL; }
    ~WvResolverHost()
        {
	    RELEASE(loop);
            if (pid && pid != -1)
            {
                kill(pid, SIGKILL);
                waitpid(pid, NULL, 0);
            }
        }
protected:
    WvResolverHost()
        { init(); }
    void init()
        { done = negative = false;
          pid = 0; loop = NULL; last_tried = time(NULL); }
};

class WvResolverAddr : public WvResolverHost
{
public:
    WvResolverAddr(WvIPAddr *_addr)
        { addr = _addr; }
};

// static members of WvResolver
int WvResolver::numresolvers = 0;
WvResolverHostDict *WvResolver::hostmap = NULL;
WvResolverAddrDict *WvResolver::addrmap = NULL;


// function that runs in a child task

static void namelookup(const char *name, WvLoopback *loop)
{
    struct hostent *he;
    
    // wait up to one minute...
    alarm(60);
    
    for (;;)
    {
	he = gethostbyname(name);
	if (he)
	{
            char **addr = he->h_addr_list;
            while (*addr != NULL)
            {
  	        loop->print("%s ", WvIPAddr((unsigned char *)(*addr)));
                addr++;
            }
            loop->print("\n");
	    alarm(0);
	    return;
	}
	
	// not found (yet?)
	
	if (h_errno != TRY_AGAIN)
	{
	    alarm(0);
	    return; // not found; blank output
	}
    }
}


WvResolver::WvResolver()
{
    numresolvers++;
    if (!hostmap)
	hostmap = new WvResolverHostDict(10);
    if (!addrmap)
	addrmap = new WvResolverAddrDict(10);
}


WvResolver::~WvResolver()
{
    numresolvers--;
    if (numresolvers <= 0 && hostmap && addrmap)
    {
	delete hostmap;
	delete addrmap;
	hostmap = NULL;
	addrmap = NULL;
    }
}


// returns >0 on success, 0 on not found, -1 on timeout
// If addr==NULL, this just tests to see if the name exists.
int WvResolver::findaddr(int msec_timeout, WvStringParm name,
			 WvIPAddr const **addr,
                         WvIPAddrList *addrlist)
{
    WvResolverHost *host;
    time_t now = time(NULL);
    int res = 0;
    
    host = (*hostmap)[name];

    if (host)
    {
	// refresh successes after 5 minutes, retry failures every 1 minute
	if ((host->done && host->last_tried + 60*5 < now)
	    || (!host->done && host->last_tried + 60 < now))
	{
	    // expired from the cache.  Force a repeat lookup below...
	    hostmap->remove(host);
	    host = NULL;
	}
	else if (host->done)
	{
	    // entry exists, is marked done, and hasn't expired yet.  Return
	    // the cached value.
	    if (addr)
		*addr = host->addr;
            if (addrlist)
            {
                WvIPAddrList::Iter i(host->addrlist);
                for (i.rewind(); i.next(); )
                {
                    addrlist->append(i.ptr(), false);
                    res++;
                }
            }
            else
                res = 1;
	    return res;
	}
	else if (host->negative)
	{
	    // the entry is in the cache, but the response was negative:
	    // the name doesn't exist.
	    return 0;
	}
	
	// if we get here, 'host' either exists (still in progress)
	// or is NULL (need to start again).
    }

    if (!host)
    {
	// nothing matches this hostname in the cache.  Create a new entry,
	// and start a new lookup.
	host = new WvResolverHost(name);
	hostmap->add(host, true);
	
	host->loop = new WvLoopback();
	
#ifdef WVRESOLVER_SKIP_FORK
        // background name resolution doesn't work when debugging with gdb!
	namelookup(name, host->loop);
#else
        // fork a subprocess so we don't block while doing the DNS lookup.

	// close everything but host->loop in the subprocess.
	host->pid = wvfork(host->loop->getrfd(), host->loop->getwfd());
	
	if (!host->pid)
	{
	    // child process
	    host->loop->noread();
	    namelookup(name, host->loop);
	    _exit(1);
	}
#endif
	
	// parent process
	host->loop->nowrite();
    }

#ifndef WVRESOLVER_SKIP_FORK
    
    // if we get here, we are the parent task waiting for the child.
    
    do
    {
	if (waitpid(host->pid, NULL, WNOHANG) == host->pid)
	    host->pid = 0;
	
	if (!host->loop->select(msec_timeout < 0 ? 100 : msec_timeout,
				true, false))
	{
	    if (host->pid)
	    {
		if (msec_timeout >= 0)
		    return -1; // timeout, but still trying
	    }
	    else
	    {
		// the child is dead.  Clean up our stream, too.
		RELEASE(host->loop);
		host->loop = NULL;
		host->negative = true;
		return 0; // exited while doing search
	    }
	}
	else
	    break;
    } while (host->pid && msec_timeout < 0); // repeat if unlimited timeout!
#endif

    // data coming in!
    char *line;
    
    do
    {
	line = host->loop->getline(-1);
    } while (!line && host->loop->isok());
    
    if (line && line[0] != 0)
    {
        res = 1;
        WvIPAddr *resolvedaddr;
        char *p;
        p = strtok(line, " \n");
	resolvedaddr = new WvIPAddr(p);
	host->addr = resolvedaddr;
        host->addrlist.append(resolvedaddr, true);
        if (addr)
            *addr = host->addr;
        if (addrlist)
            addrlist->append(host->addr, false);
        do
        {
            p = strtok(NULL, " \n");
            if (p)
            {
                res++;
                resolvedaddr = new WvIPAddr(p);
                host->addrlist.append(resolvedaddr, true);
                if (addrlist)
                    addrlist->append(resolvedaddr, false);
            }
        } while (p);
	host->done = true;
    }
    else
	host->negative = true;

    if (host->pid && waitpid(host->pid, NULL, 0) == host->pid)
	host->pid = 0;
    RELEASE(host->loop);
    host->loop = NULL;
    
    // Return as many addresses as we find.
    return host->negative ? 0 : res;
}

void WvResolver::clearhost(WvStringParm hostname)
{
    WvResolverHost *host = (*hostmap)[hostname];
    if (host)
        hostmap->remove(host);
}

/*
int WvResolver::findname(int msec_timeout, WvIPAddr *ipaddr, char **name)
{
    fprintf(stderr, "FIXME: WvResolver::findname() not implemented!\n");
    return 0;
}
*/


bool WvResolver::pre_select(WvStringParm hostname,
			      WvStream::SelectInfo &si)
{
    WvResolverHost *host = (*hostmap)[hostname];
    
    if (host)
    {
	if (host->loop)
	    return host->loop->xpre_select(si,
			  WvStream::SelectRequest(true, false, false));
	else
	    return true; // sure thing: already looked up this name!
    }
    else
	return false; // will never be ready... host not even in map!
}
