/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2004 Net Integration Technologies, Inc.
 *
 * OpenSLP Service Lister
 */

#include <string.h>

#include "wvstringlist.h"
#include "slp.h"

#ifdef WITH_SLP

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

bool slp_get_servs(WvStringParm service, WvStringList &servlist)
{
    SLPError slperr; 
    SLPHandle hslp;

    servlist.zap();
    
    slperr = SLPOpen("en",SLP_FALSE, &hslp); 
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

#endif
