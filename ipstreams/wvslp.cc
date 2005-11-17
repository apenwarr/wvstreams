/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2004 Net Integration Technologies, Inc.
 *
 * OpenSLP Service Lister
 */

#include <string.h>

#include "wvstringlist.h"
#include "wvslp.h"



#ifdef WITH_SLP
#include "slp.h"

// Note - this only LOOKS Asynchronous - the OpenSLP folks haven't
// finished their Async code yet, so for now, we're stuck with a nice
// blocking call...
// 
static SLPBoolean slpcb(SLPHandle hslp, const char *srvurl, 
		  unsigned short lifetime, SLPError errcode, 
		  void *cookie)
{
    printf("Going Into the callback!\n");
    printf("srvurl is: %s\n", srvurl);
    if (errcode == SLP_OK || errcode == SLP_LAST_CALL)
    {
	WvStringList &list = *(WvStringList *)cookie;
	// srvurl is of the form:
	// service:servicename://server:port
	if (srvurl && strrchr(srvurl, '/'))
	{
	    WvString server(strdup(strrchr(srvurl, '/'))+1);
	    printf("Found: %s\n", server.cstr());
	    list.append(server);
	}
	else
	{
	    if (!srvurl)
		printf("End of the list!\n");
	    else
		printf("Malformed URL: %s\n", srvurl);
	}
    }
    else
    {
	printf("What happened??\n");
    }
    printf("Coming out of the callback!\n");
    return SLP_TRUE;
}

static SLPBoolean slpacb(SLPHandle hslp, const char* pcAttrList, 
			 SLPError errcode, void *cookie)
{
    if (errcode == SLP_OK)
    {
    	WvString attrval = *(WvString *)cookie;
    	if (pcAttrList)
    	{
    	    attrval = pcAttrList;
    	    printf("Found: %s\n", attrval.cstr());
    	}
    	return SLP_TRUE;
    }
    return SLP_FALSE;
}

bool slp_get_servs(WvStringParm service, WvStringList &servlist)
{
    SLPError slperr; 
    SLPHandle hslp;

    servlist.zap();
    
    slperr = SLPOpen("en", SLP_FALSE, &hslp); 
    if(slperr != SLP_OK) 
    { 
        printf("Error opening slp handle\n");
        return false; 
    } 

    slperr = SLPFindSrvs(hslp, service, NULL, NULL, slpcb, &servlist);
    if (slperr != SLP_OK)
    {
	printf("Something went wrong finding the service.");
	printf("You may have an incomplete list!");
    }
    
    SLPClose(hslp);
    printf("Ok - got %d servers, returning...\n", servlist.count());
    return true;
}

bool slp_get_attrs(WvStringParm service, WvStringParm attribute, 
		   WvStringList &servlist)
{
    SLPError slperr;
    SLPHandle hslp;

    servlist.zap();

    slperr = SLPOpen("en", SLP_FALSE, &hslp); 
    if(slperr != SLP_OK) 
    { 
        printf("Error opening slp handle\n");
        return false; 
    }
    
    WvStringList possibleservers;
    slperr = slp_get_servs(service, possibleservers);
    
    WvStringList::Iter i(possibleservers);
    for (i.rewind(); i.next(); )
    {
	WvString attrval;
	slperr = SLPFindAttrs(hslp, WvString("service:%s://%s", service, *i), 
			      NULL, attribute, slpacb, &attrval);
    	if (slperr == SLP_OK)
	{
	    WvString value = strrchr(attrval.edit(), '=');
	    servlist.append(new WvString("%s,%s", *i, value), true);
	}
	else
	    continue;
    }

    SLPClose(hslp);
    printf("Ok - got %d servers, returning...\n", servlist.count());
    return true;
}

// Use the below for servers that want to advertise via SLP

static void sillyslpcb(SLPHandle hslp, SLPError errcode, void* cookie)
{ 
    /* return the error code in the cookie */ 
    *(SLPError*)cookie = errcode; 
}

WvSlp::WvSlp()
    : log("WvSlp", WvLog::Info)
{
    SLPError slperr = SLPOpen("en", SLP_FALSE, &hslp);
    if(slperr != SLP_OK)
    { 
        log(WvLog::Critical, "Error opening SLP handle: %s\n", slperr);
        err.seterr("SLP Startup Broken: %s", slperr);
    }
}

WvSlp::~WvSlp()
{
    SLPError callbackerr;

    WvStringList::Iter i(services);
    
    for (i.rewind(); i.next(); )
	SLPDereg(hslp, *i, sillyslpcb, &callbackerr);
    
    SLPClose(hslp);
    services.zap();
}

void WvSlp::add_service(WvStringParm name, WvStringParm hostname, WvStringParm port)
{
    SLPError callbackerr;
    
    WvString *svc = new WvString("service:%s://%s:%s", name, hostname, port);
    SLPError slperr = SLPReg(hslp, *svc, SLP_LIFETIME_MAXIMUM, 0, "", SLP_TRUE,
			     sillyslpcb, &callbackerr);
    
    if(slperr != SLP_OK)
    { 
	log(WvLog::Notice, "Error registering %s: %s\n", *svc, slperr);
	err.seterr("SLP Registration Broken: %s", slperr);
    }
    else
	services.add(svc, true);
}

#else
bool slp_get_servs(WvStringParm service, WvStringList &servlist)
{ 
    return true;
}

bool slp_get_attrs(WvStringParm service, WvStringParm attribute, 
		   WvStringList &servlist)
{
    return true;
}

WvSlp::WvSlp()
    : log("WvSlp", WvLog::Info)
{
    log("WvSlp compiled without SLP Library..\n");
    log("Not registering\n");
}

WvSlp::~WvSlp()
{
}

void WvSlp::add_service(WvStringParm name, WvStringParm hostname, WvStringParm port)
{
    
}
#endif
