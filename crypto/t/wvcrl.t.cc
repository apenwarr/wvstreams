#include "wvtest.h"
#include "wvcrl.h"
#include "wvfile.h"
#include "wvfileutils.h"
#include "wvx509mgr.h"

// default keylen for where we're not using pre-existing certs
const static int DEFAULT_KEYLEN = 512; 

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


WVTEST_MAIN("decoding and encoding basics")
{
    WvCRL crl;
    
    crl.decode(WvCRL::CRLPEM, crl_pem);
    WVPASSEQ(crl.get_aki(), 
             "42:1B:6F:97:0B:23:79:1F:C1:08:57:9E:3A:A6:0A:9C:90:88:1F:D8");
    WVPASSEQ(crl.get_issuer(), "/C=US/O=Test Certificates/CN=Bad Signed CA");
    
    WvString s = crl.encode(WvCRL::CRLPEM);
    WVPASSEQ(s, crl_pem);

    WvDynBuf d;
    crl.encode(WvCRL::CRLDER, d);
    WVPASSEQ(d.used(), 318); // size of der encoded certificate
    WvCRL crl2;
    crl2.decode(WvCRL::CRLDER, d);
    WVPASSEQ(crl.get_aki(), crl2.get_aki());
}


WVTEST_MAIN("CRL creation and use basics")
{
    WvX509Mgr ca("cn=testca.ca,dc=testca,dc=ca", DEFAULT_KEYLEN, true);
    WvCRL crl(ca);
    WVPASSEQ(crl.numcerts(), 0);
    WVPASSEQ(crl.get_issuer(), ca.get_subject());

    WvRSAKey rsakey(DEFAULT_KEYLEN);
    WvString certreq = WvX509Mgr::certreq("cn=test.signed.com,dc=signed,dc=com", 
                                       rsakey);
    WvString srequest = ca.signreq(certreq);
    WvX509 user;
    user.decode(WvX509::CertPEM, srequest);
    WVFAIL(crl.isrevoked(user));
    WVFAIL(crl.isrevoked(user.get_serial()));

    crl.addcert(user);
    WVPASSEQ(crl.numcerts(), 1);
    WVPASS(crl.isrevoked(user));
    WVPASS(crl.isrevoked(user.get_serial()));
}


static bool exists(WvStringParm filename)
{
    return access(filename, F_OK) == 0;
}


bool test_encode_load_file(WvCRL::DumpMode mode)
{
    WvString tmpfile = wvtmpfilename("crl");

    WvX509Mgr ca("o=ca", DEFAULT_KEYLEN, true);
    WvCRL crl1(ca);

    WVPASS(exists(tmpfile));
    {
        WvFile f(tmpfile, O_WRONLY);
        WvDynBuf buf;
        crl1.encode(mode, buf);
        f.write(buf, buf.used());
    }
    WVPASS(exists(tmpfile));

    WvCRL crl2;
    if (mode == WvCRL::CRLPEM)
        crl2.decode(WvCRL::CRLFilePEM, tmpfile);
    else
        crl2.decode(WvCRL::CRLFileDER, tmpfile);

    ::unlink(tmpfile);
    WVFAIL(exists(tmpfile));

    WVPASSEQ(crl1.get_issuer(), crl2.get_issuer());
    return (crl1.get_issuer() == crl2.get_issuer());
}


WVTEST_MAIN("Loading/unloading from file")
{
    WVPASS(test_encode_load_file(WvCRL::CRLPEM));
    WVPASS(test_encode_load_file(WvCRL::CRLDER));
}
