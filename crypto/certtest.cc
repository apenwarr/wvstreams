#include "wvx509.h"
#include "wvlog.h"

#include "pem.h"
#include "x509v3.h"

// Quick program to test the certificate generation routines
// from WvX509Mgr. Take the output of PEM_write_X509, and run it through
// openssl x509 -text
// (The part between ----BEGIN CERTIFICATE---- and ----END CERTIFICATE---- )

int main()
{
	free(malloc(1)); // For Electric Fence...

	WvLog log("certtest", WvLog::Info);
	log("Starting...\n");
	
	// Setup a new DN entry, like a server would set.
	WvString dN("cn=test.foo.com,dc=foo,dc=com");
	// Create a new certificate
	WvX509Mgr x509cert(dN,1024);
	
	if (!x509cert.err)
	{
	    WvString filename("testcert.pem");
	    x509cert.dumpkeypair(filename,false);
	    x509cert.dumpcert(filename,true);
	}
	else
		log("Error: %s",x509cert.errstr);

	log("Done...\n");
}