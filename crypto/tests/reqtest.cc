#include "wvrsa.h"
#include "wvx509.h"
#include "wvlog.h"

// Quick program to test the certificate request generation routines
// from WvX509Mgr. Output should be put through:
// openssl req -text
// (The part between ----BEGIN CERTIFICATE REQUEST---- and 
// ----END CERTIFICATE REQUEST---- )

int main(int argc, char **argv)
{
    free(malloc(1)); // For Electric Fence...
    
    WvString request;
    
    WvLog log("reqtest", WvLog::Info);
    log("Starting...\n");
    
    // Setup a new DN entry, like a server would set.
    WvString dn(argc > 1 ? argv[1] : "cn=test.foo.com,dc=foo,dc=com");
    
    // Setup a key
    WvRSAKey rsa(1024);

    // Create a new certificate
    WvString certreq = WvX509Mgr::certreq(dn, rsa);
    
    log("Private RSA key follows (KEEP THIS!):\n");
    wvcon->write(rsa.getpem(true));
    
    log("Certificate request follows:\n");
    wvcon->write(certreq);
    
    log("Done...\n");
}
