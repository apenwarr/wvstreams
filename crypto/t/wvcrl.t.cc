#include "wvtest.h"
#include "wvcrl.h"
#include "wvx509.h"
#include "wvhex.h"

const static char crl_pem[] = 
"-----BEGIN X509 CRL-----\n"
"MIIBOjCBpAIBATANBgkqhkiG9w0BAQUFADBBMQswCQYDVQQGEwJVUzEaMBgGA1UE\n"
"ChMRVGVzdCBDZXJ0aWZpY2F0ZXMxFjAUBgNVBAMTDUJhZCBTaWduZWQgQ0EXDTAx\n"
"MDQxOTE0NTcyMFoXDTExMDQxOTE0NTcyMFqgLzAtMB8GA1UdIwQYMBaAFEIbb5cL\n"
"I3kfwQhXnjqmCpyQiB/YMAoGA1UdFAQDAgEBMA0GCSqGSIb3DQEBBQUAA4GBABGR\n"
"fRmlA06n8R8YYpT6l6BGQZYxlRlJeanl8YSqyNzIf5IH5PxmlpsujnOqrXARGf2O\n"
"vvJ0P3kCWuYqZ7RG1x2K3sjQt8z2hduQpkacrSMJR6rwREVj9UDd/HW/S4Uv+nQJ\n"
"+BmPKZeSrjRcbmtrQHRRWCbflbdyEz2xdpoKQzcx\n"
"-----END X509 CRL-----\n";


WVTEST_MAIN("basics")
{
    WvCRL crl;
    
    crl.decode(WvCRL::PEM, crl_pem);
    WVPASSEQ(crl.get_aki(), 
             "42:1B:6F:97:0B:23:79:1F:C1:08:57:9E:3A:A6:0A:9C:90:88:1F:D8");
    WVPASSEQ(crl.get_issuer(), "/C=US/O=Test Certificates/CN=Bad Signed CA");
}

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
