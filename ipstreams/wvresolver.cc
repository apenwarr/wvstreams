/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2001 Net Integration Technologies, Inc.
 * 
 * DNS name resolver with support for background lookups.
 */
#include "wvresolver.h"
#include "wvloopback.h"
#include "wvaddr.h"
#include "wvhashtable.h"
#include "wvtcp.h"
#include "wvfork.h"
#include <netdb.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

class WvResolverHost
{
public:
    WvString name;
    WvIPAddr *addr;
    bool done, negative;
    pid_t pid;
    WvLoopback *loop;
    time_t last_tried;
    
    WvResolverHost(const WvString &_name) : name(_name)
        { init(); name.unique(); addr = NULL; }
    ~WvResolverHost()
        {   
	    if (addr) delete addr;
	    if (loop) delete loop; 
	    if (pid)
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

DeclareWvDict(WvResolverHost, WvString, name);
DeclareWvDict(WvResolverAddr, WvIPAddr, addr[0]);

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
	    loop->print("%s\n", WvIPAddr((unsigned char *)he->h_addr));
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
int WvResolver::findaddr(int msec_timeout, const WvString &name,
			 WvIPAddr const **addr)
{
    WvResolverHost *host;
    time_t now = time(NULL);

    host = (*hostmap)[name];
    if (host)
    {
	// refresh successes after 5 minutes, retry failures every 1 minute
	if ((host->done && host->last_tried + 60*5 < now)
	    || (!host->done && host->last_tried + 60 < now))
	{
	    hostmap->remove(host);
	    host = NULL;
	}
	else if (host->done)
	{
	    if (addr)
		*addr = host->addr;
	    return 1;
	}
	else if (host->negative)
	    return 0;
    }
    
    if (!host)
    {
	host = new WvResolverHost(name);
	hostmap->add(host, true);
	
	host->loop = new WvLoopback();
#define SKIP_FORK_FOR_DEBUGGING 0 // this does NOT work on a real weaver!!
#if SKIP_FORK_FOR_DEBUGGING
	namelookup(name, host->loop);
#else
	host->pid = wvfork(host->loop->getfd());    // don't close host->loop!
	
	if (!host->pid) // child process
	{
	    host->loop->noread();
	    namelookup(name, host->loop);
	    _exit(1);
	}
	host->loop->nowrite();
#endif
    }
    
    // if we get here, we are the parent task waiting for the child.
    
    do
    {
	if (waitpid(host->pid, NULL, WNOHANG) == host->pid)
	    host->pid = 0;
	
	if (!host->loop->select(msec_timeout < 0 ? 100 : msec_timeout))
	{
	    if (host->pid)
	    {
		if (msec_timeout >= 0)
		    return -1; // timeout, but still trying
	    }
	    else
	    {
		delete host->loop;
		host->loop = NULL;
		host->negative = true;
		return 0; // exited while doing search
	    }
	}
	else
	    break;
    } while (host->pid && msec_timeout < 0); // repeat if unlimited timeout!
    
    // data coming in!
    char *line;
    
    do
    {
	line = host->loop->getline(-1);
    } while (!line && host->loop->isok());
    
    if (line && line[0] != 0)
    {
	host->addr = new WvIPAddr(line);
	host->done = true;
    
	if (addr)
	    *addr = host->addr;
    }
    else
	host->negative = true;

    if (host->pid && waitpid(host->pid, NULL, 0) == host->pid)
	host->pid = 0;
    delete host->loop;
    host->loop = NULL;
    
    // laziness: we always assume just one address.
    return host->negative ? 0 : 1;
}

/*
int WvResolver::findname(int msec_timeout, WvIPAddr *ipaddr, char **name)
{
    fprintf(stderr, "FIXME: WvResolver::findname() not implemented!\n");
    return 0;
}
*/


bool WvResolver::select_setup(const WvString &hostname,
			      WvStream::SelectInfo &si)
{
    WvResolverHost *host = (*hostmap)[hostname];
    
    if (host)
    {
	if (host->loop)
	    return host->loop->select_setup(si);
	else
	    return true; // sure thing: already looked up this name!
    }
    else
	return false; // will never be ready... host not even in map!
}
