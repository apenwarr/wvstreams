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
    
    {
	WvX509Mgr *x509cert = new WvX509Mgr("cn=test.org", 1024);
	if (x509cert->test())
	    log("New certificate tests out okay.\n");
	else
	    log(WvLog::Error, "New certificate failed the self-test!\n");
    }
    
    // Create a new certificate
    WvX509Mgr x509cert(dN, 1024);
    
    if (x509cert.isok())
    {
	wvcon->write(x509cert.encode(WvX509Mgr::CertPEM));
	wvcon->write(x509cert.encode(WvX509Mgr::RsaPEM));
	wvcon->write(x509cert.encode(WvX509Mgr::RsaRaw));
    }
    else
	log("Error: %s\n", x509cert.errstr());
    
    log("Test result: %s\n", x509cert.test());
    
    log("Done...\n");
}
