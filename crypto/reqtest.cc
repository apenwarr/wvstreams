#include "wvx509.h"
#include "wvlog.h"

// Quick program to test the certificate request generation routines
// from WvX509Mgr. Output should be put through:
// openssl req -text
// (The part between ----BEGIN CERTIFICATE REQUEST---- and 
// ----END CERTIFICATE REQUEST---- )

int main()
{
	free(malloc(1)); // For Electric Fence...

	WvX509Mgr *x509mgr = new WvX509Mgr();
	WvString request;

	WvLog log("reqtest", WvLog::Info);
	log("Starting...\n");
	
	// Setup a new DN entry, like a server would set.
	WvString dN("cn=test.foo.com,dc=foo,dc=com");
	// Create a new certificate
	request = x509mgr->createcertreq(dN,1024);
	
	if (!!request)
		log(request);
	else
		log("Failed to generate certificate");

	log("Done...\n");
}
