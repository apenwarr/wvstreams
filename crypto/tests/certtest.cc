#include "wvx509.h"
#include "wvlog.h"

#include <openssl/pem.h>
#include <openssl/x509v3.h>

// Quick program to test the certificate generation routines
// from WvX509Mgr. Take the output of .encode(WvX509Mgr::CertPEM), 
// and run it through:
// openssl x509 -text
// (The part between ----BEGIN CERTIFICATE---- and ----END CERTIFICATE---- )
// 
// To test the PKCS12 routines: 
// openssl  pkcs12 -in /tmp/test.p12 -nodes
// 
// Which should give you a PEM Encoded Certificate, a PEM Encoded RSA Private Key,
// and a bunch of other crap ;)
// 

int main()
{
    free(malloc(1)); // For Electric Fence...
    
    WvLog log("certtest", WvLog::Info);
    log("Starting...\n");
    
    // Setup a new DN entry, like a server would set.
    WvString dN("cn=test.foo.com,dc=foo,dc=com");
    
    // Create a new certificate
    WvX509Mgr x509cert(dN, 1024);
    x509cert.setPkcs12Password("Foo");
    
    log("Consistancy Test result: %s\n", x509cert.test());

    if (x509cert.isok())
    {
	wvcon->write(x509cert.encode(WvX509Mgr::CertPEM));
	wvcon->write(x509cert.encode(WvX509Mgr::RsaPEM));
	wvcon->write(x509cert.encode(WvX509Mgr::RsaRaw));
	x509cert.write_p12("/tmp/test.p12");
    }
    else
	log("Error: %s\n", x509cert.errstr());
    
    log("Errors after the write: %s\n", x509cert.errstr());

    log("Done...\n");
}
