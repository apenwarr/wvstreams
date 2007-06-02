#include "wvx509.h"
#include "wvrsa.h"
#include "wvlog.h"
#include "strutils.h"
#include "wvcrash.h"

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

void test(WvStringParm _dN)
{
    // Create a new certificate
    WvX509Mgr x509cert;
    x509cert.set_dname(_dN);
    x509cert.set_rsakey(new WvRSAKey(1024));
    x509cert.create_selfsigned(true);
    wvcon->print("Consistancy Test result: %s\n", 
		 x509cert.test() ? "Ok" : "Inconsistant");

    if (x509cert.isok())
    {
	wvcon->print(x509cert.encode(WvX509Mgr::CertPEM));
	wvcon->print(x509cert.encode(WvX509Mgr::RsaPEM));
	wvcon->print(x509cert.encode(WvX509Mgr::RsaRaw));
	x509cert.write_p12("/tmp/test.p12", "foo");
	wvcon->print("Private Key: %s\n", x509cert.get_rsa().private_str());
	wvcon->print("Certificate: %s\n", x509cert.hexify());
    }
    else
	wverr->print("Error: %s\n", x509cert.errstr());

    // check and make sure that the PKCS12 wrote properly...
    if (!x509cert.isok())
        wverr->print("Errors after the write: %s\n", x509cert.errstr());
    
}

int main(int argc, char *argv[])
{
    wvcrash_setup(argv[0]);

    free(malloc(1)); // For Electric Fence...
    
    wvcon->print("Certificate Test Starting...\n");
    
    WvString dName("cn=Test CA, o=TESTING DO NOT USE");
    
    test(dName);

    // Or, from the actual settings of the server...
    // this tests the case where the domainname() ends up 
    // being (none)
    dName = encode_hostname_as_DN(fqdomainname());

    test(dName);

    wvcon->print("Certificate Test Done...\n");
}
