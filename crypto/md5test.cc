#include "wvcrypto.h"
#include "wvlog.h"

// Quick program to test the MD5 Routines...

int main()
{
	free(malloc(1)); // For Electric Fence...

	WvString request = "Hello World";
	WvString hash;

	WvLog log("MD5test", WvLog::Info);
	log("Starting...\n");
	
	WvMD5 md5test(request);

        log("%s -> %s\n",request, md5test.MD5Hash());

	log("Done...\n");
}
