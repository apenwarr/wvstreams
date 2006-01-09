#include "wvtest.h"
#include "wvcrl.h"
#include "wvx509.h"
#include "wvhex.h"

#if 0 // BUGZID: 17793
WVTEST_MAIN("CRL")
{
    WvCRLMgr crl;
    WVFAIL(crl.isok());
    
    WvX509Mgr ca("o=ca", 1024);
    ca.create_selfsigned(true);
    crl.setca(&ca);
    WVPASS(crl.numcerts() == 0);
    WvX509Mgr user("cn=user,o=ca", 1024);
    WvString request = user.certreq();
    WvString srequest = ca.signreq(request);
    user.decode(WvX509Mgr::CertPEM, srequest);
    WVFAIL(crl.isrevoked(&user));
    WVFAIL(crl.isrevoked(user.get_serial()));
    crl.addcert(&user);
    WVPASS(crl.numcerts() == 1);
    WVPASS(crl.isrevoked(&user));
    WVPASS(crl.isrevoked(user.get_serial()));
//    WvString crl_pem = crl.encode(WvCRLMgr::CRLPEM);
}
#endif
