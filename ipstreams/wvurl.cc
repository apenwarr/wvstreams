/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * WvUrl is a simple URL-parsing class with built-in (though still somewhat
 * inconvenient) DNS resolution.
 * 
 * See wvurl.h.
 */ 
#include "wvurl.h"

// A static list of the default ports for each protocol.
struct DefaultPort
{
    char *proto;
    int port;
    bool uses_slashes;
};

// The protocols must be arranged from longest to shortest because they're
// compared with strncmp, so "https://" will also match http.
DefaultPort portmap[] = {
    { "https", 443, true },
    { "http", 80, true },
    { "sip", 5060, false },
    { NULL, 0 }
};

// Look up the protocol and return the default port.
int get_default_port(WvString proto)
{
    DefaultPort *p = portmap;
    for (p = portmap; p->proto != NULL; p++)
    {
        if (strncmp(p->proto, proto, strlen(p->proto)) == 0)
            return p->port;
    }
    return -1;
}

// Look up the protocol and decide whether it uses slashes (http) or not (sip)
// A check of rfc2396 shows that the URI standard actually distinguishes
// these: 'hierarchical' vs. 'opaque'.
bool protocol_uses_slashes(WvString proto)
{
    DefaultPort *p = portmap;
    for (p = portmap; p->proto != NULL; p++)
    {
        if (strncmp(p->proto, proto, strlen(p->proto)) == 0)
            return p->uses_slashes;
    }
    return false;
}

// Split up the URL into a hostname, a port, and the rest of it.
WvUrl::WvUrl(WvStringParm url) : err("No error")
{
    WvString work(url);
    char *cptr, *wptr = work.edit();
    
    port = 0; // error condition by default
    addr = NULL;
    resolving = true;

    // if it's not one of these easy prefixes, give up.  Our URL parser is
    // pretty dumb.
    if (get_default_port(wptr) < 0)
    {
	err = "WvUrl cannot handle the given protocol.";
	return;
    }

    cptr = strchr(wptr, ':');
    if (!cptr)
    {
        err = "No colon after the protocol.";
        return;
    }
    *cptr = 0;
    proto = wptr;

    bool use_slashes = protocol_uses_slashes(proto);
    wptr = cptr + (use_slashes ? 3 : 1);

    cptr = strchr(wptr, '@');
    if (!cptr) // no user given
        user = "";
    else
    {
        *cptr = 0;
        user = wptr;
        wptr = cptr + 1;
    }
    
    cptr = strchr(wptr, '/');
    if (!cptr) // no path given
	file = use_slashes ? "/" : "";
    else
    {
	file = cptr;
	*cptr = 0;
    }
    
    cptr = strchr(wptr, ':');
    if (!cptr)
	port = get_default_port(proto);
    else
    {
	port = atoi(cptr+1);
	*cptr = 0;
    }

    hostname = wptr;

    resolve();
}


WvUrl::WvUrl(const WvUrl &url) : err("No error")
{
    addr = NULL;
    resolving = true;
    
    proto = url.proto;
    user = url.user;
    hostname = url.hostname;
    file = url.file;
    port = url.port;

    resolve();
}


WvUrl::~WvUrl()
{
    if (addr) delete addr;
}


bool WvUrl::resolve()
{
    const WvIPAddr *ip;
    int numaddrs;
    
    numaddrs = dns.findaddr(0, hostname, &ip);
    if (!numaddrs) // error condition
    {
	err = WvString("Host '%s' could not be found.", hostname);
	resolving = false;
	return false;
    }
    else if (numaddrs < 0) // still waiting
    {
	resolving = true;
	return false;
    }
    else // got at least one address
    {
	resolving = false;
	if (addr) delete addr;
	addr = new WvIPPortAddr(*ip, port);
	return true;
    }
}


// Print out the URL, using the port name (if it's not 80), and either the 
// hostname (if we know it) or the address (if we know that instead.)
WvUrl::operator WvString () const
{
    if (!isok())
	return WvString("(Invalid URL: %s)", err);

    WvString protostr;
    if (protocol_uses_slashes(proto))
        protostr = WvString("%s://", proto);
    else
        protostr = WvString("%s:", proto);
    WvString userstr("");
    if (user && user.len() != 0)
        userstr = WvString("%s@", user);
    WvString portstr("");
    if (port && port != get_default_port(proto))
	portstr = WvString(":%s", port);
    if (hostname)
	return WvString("%s%s%s%s%s", protostr, userstr, hostname, portstr, file);
    else if (addr)
	return WvString("%s%s%s%s%s", protostr, userstr, *addr, portstr, file);
    else
    {
	assert(0);
	return WvString("(Invalid URL)");
    }
}


