#include "wvfile.h"
#include "wvocsp.h"
#include "wvtest.h"
#include "wvx509mgr.h"
#include "wvfileutils.h"
#include "wvrsa.h"
#include "wvsystem.h"

#ifndef _WIN32 // the way we call openssl's ocsp util will not work on win32

// default keylen for where we're not using pre-existing certs
const static int DEFAULT_KEYLEN = 512; 

// for objects with only one kind of encoding (e.g. ocsp requests/replies)
#define ENCODE_TO_FILE(FNAME, OBJ)                                      \
    {                                                                   \
        WvDynBuf buf;                                                   \
        OBJ.encode(buf);                                                \
        WvFile f(FNAME, O_CREAT|O_WRONLY);                              \
        f.write(buf);                                                   \
    }

// for objects with multiple encodings (e.g. certs)
#define ENCODE_TO_FILE2(FNAME, OBJ, FORMAT)                             \
    {                                                                   \
        WvDynBuf buf;                                                   \
        OBJ.encode(FORMAT, buf);                                        \
        WvFile f(FNAME, O_CREAT|O_WRONLY);                              \
        f.write(buf);                                                   \
    }

// dumping some stuff to file
#define DUMP_TO_FILE(FNAME, STUFF)                                      \
    {                                                                   \
        WvFile f(FNAME, O_CREAT|O_WRONLY);                              \
        f.print(STUFF);                                                 \
    }


static WvOCSPResp::Status test_ocsp_req(WvX509 &cert, WvX509Mgr &cacert, 
                                        WvStringParm indexcontents)
{
    WvString reqfname = wvtmpfilename("ocspreq");
    WvString respfname = wvtmpfilename("ocspresp");
    WvString indexfname = wvtmpfilename("ocspidx");    
    WvString cafname = wvtmpfilename("ocspca");
    WvString cakeyfname = wvtmpfilename("ocspcakey");
    WvString clifname = wvtmpfilename("ocspcli");

    ENCODE_TO_FILE2(cafname, cacert, WvX509::CertPEM);
    ENCODE_TO_FILE2(clifname, cert, WvX509::CertPEM);
    ENCODE_TO_FILE2(cakeyfname, cacert, WvRSAKey::RsaPEM);
    DUMP_TO_FILE(indexfname, indexcontents);

    WvOCSPReq req(cert, cacert);
    ENCODE_TO_FILE(reqfname, req);

    WvSystem("openssl", "ocsp", "-CAfile", cafname, "-index", indexfname, "-rsigner", 
             cafname, "-rkey", cakeyfname, "-CA", cafname, 
             "-reqin", reqfname, "-respout", respfname, "-text");

    WvOCSPResp resp(cacert, cacert); // issuer is ocsp server in these tests
    {
        WvFile f(respfname, O_RDONLY);
        WvDynBuf buf;
        f.read(buf, INT_MAX);
        resp.decode(buf);
    }

    ::unlink(reqfname);
    ::unlink(respfname);
    ::unlink(indexfname);
    ::unlink(cafname);
    ::unlink(cakeyfname);

    return resp.get_status(req);
}


WVTEST_MAIN("encoding request")
{
    WvRSAKey rsakey(DEFAULT_KEYLEN);
    WvString certreq
	= WvX509Mgr::certreq("cn=test.signed.com,dc=signed,dc=com", rsakey);

    WvX509Mgr cacert("CN=test.foo.com,DC=foo,DC=com", DEFAULT_KEYLEN, true);
    // cacert.set_ext_key_usage("OCSPSigning");
    // cacert.signcert(cacert); // resign self


    WvX509Mgr cert;
    WvString certpem = cacert.signreq(certreq);
    cert.decode(WvX509Mgr::CertPEM, certpem);
    cert.set_rsa(&rsakey);
    ENCODE_TO_FILE2("/tmp/clicert", cert, WvX509::CertPEM);

    // this test breaks after 2020. oh well.
    static const char *EXPDATE = "201210194703Z"; //dec 10, 2049
    static const char *REVDATE = "071211195254Z"; //dec 11 2007

    WVPASSEQ(test_ocsp_req(cert, cert, ""), WvOCSPResp::UNKNOWN);
    WVPASSEQ(test_ocsp_req(cert, cacert, ""), WvOCSPResp::UNKNOWN);
    WVPASSEQ(test_ocsp_req(cert, cacert, 
                           WvString("R\t%s\t%s\t%s\tunknown\t%s\n",
                                    EXPDATE, REVDATE, cert.get_serial(true),
                                    cert.get_subject())), 
             WvOCSPResp::REVOKED);
    WVPASSEQ(test_ocsp_req(cert, cacert, 
                           WvString("V\t%s\t%s\t%s\tunknown\t%s\n",
                                    EXPDATE, REVDATE, cert.get_serial(true),
                                    cert.get_subject())), 
             WvOCSPResp::GOOD);
    
}

#endif // _WIN32
