#include "wvcrl.h"
#include "wvx509.h"
#include "wvhex.h"
#include "wvcrash.h"

int main(int argc, char **argv)
{
    wvcrash_setup(argv[0]);    

    WvCRLMgr crl;
    crl.isok();
    
    WvX509Mgr ca("o=ca", 1024);
    ca.create_selfsigned(true);
    crl.setca(&ca);
    fprintf(stderr,"\n\n\nFoo\n\n\n\n");
    crl.numcerts();
    WvX509Mgr user("cn=user,o=ca", 1024);
    WvString request = user.certreq();
    fprintf(stderr,"Request:\n%s\n", request.cstr());
    WvString srequest = ca.signcert(request);
    fprintf(stderr,"Got past the signing bit\n");
    user.decode(WvX509Mgr::CertPEM, srequest);   
    fprintf(stderr,"Imported the certificate\n");
    crl.isrevoked(&user);
    fprintf(stderr,"Checking that new cert isn't revoked\n");
    crl.isrevoked(user.get_serial());
    fprintf(stderr,"Still isn't revoked!\n");
    crl.addcert(&user);
    fprintf(stderr,"Ok - just added it");
    crl.numcerts();
    fprintf(stderr,"And now, the number of certs should be 1\n");
    crl.isrevoked(&user);
    fprintf(stderr,"And user cert should be revoked\n");
    crl.isrevoked(user.get_serial());
    fprintf(stderr,"And should still be revoked!\n"); 

    return 0;
}
