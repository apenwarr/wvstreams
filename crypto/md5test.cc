#include "wvcrypto.h"
#include "wvlog.h"

// Quick program to test the MD5 Routines...

int main()
{
	free(malloc(1)); // For Electric Fence...

	WvString request = "md5test";

	WvLog log("MD5test", WvLog::Info);
	log("Starting...\n");
	
        log("%s -> %s\n",request, WvMD5(request));

	log("Done...\n");
}
