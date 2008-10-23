#include "wvfile.h"
#define private public
#include "wvocsp.h"
#include "wvtest.h"
#include "wvx509mgr.h"
#undef private
#include "wvfileutils.h"
#include "wvrsa.h"
#include "wvsystem.h"

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
                                        WvX509Mgr &ocspcert,
                                        WvStringParm indexcontents)
{
    WvString reqfname = wvtmpfilename("ocspreq");
    WvString respfname = wvtmpfilename("ocspresp");
    WvString indexfname = wvtmpfilename("ocspidx");    
    WvString cafname = wvtmpfilename("ocspca");
    WvString cakeyfname = wvtmpfilename("ocspcakey");
    WvString ocspfname = wvtmpfilename("ocspresp");
    WvString ocspkeyfname = wvtmpfilename("ocsprespkey");
    WvString clifname = wvtmpfilename("ocspcli");

    ENCODE_TO_FILE2(clifname, cert, WvX509::CertPEM);
    ENCODE_TO_FILE2(cafname, cacert, WvX509::CertPEM);
    ENCODE_TO_FILE2(ocspfname, ocspcert, WvX509::CertPEM);
    ENCODE_TO_FILE2(cakeyfname, cacert, WvRSAKey::RsaPEM);
    ENCODE_TO_FILE2(ocspkeyfname, ocspcert, WvRSAKey::RsaPEM);
    DUMP_TO_FILE(indexfname, indexcontents);

    WvOCSPReq req(cert, cacert);
    ENCODE_TO_FILE(reqfname, req);

    WvSystem("openssl", "ocsp", "-CAfile", cafname, "-index", indexfname, 
             "-rsigner", ocspfname, "-rkey", ocspkeyfname, "-CA", cafname, 
             "-reqin", reqfname, "-respout", respfname);

    WvOCSPResp resp; 
    {
        WvFile f(respfname, O_RDONLY);
        WvDynBuf buf;
        while (f.isok())
            f.read(buf, 1024);
        resp.decode(buf);
    }

    if (WVPASS(resp.isok()))
    {
        WVPASS(resp.check_nonce(req));

        WvX509 signing_cert = resp.get_signing_cert();
        WVPASS(signing_cert.isok());
        WVPASSEQ(signing_cert.get_subject(), ocspcert.get_subject());

        WVPASS(resp.signedbycert(ocspcert));
        WVFAIL(resp.signedbycert(cert)); 
    }

    ::unlink(reqfname);
    ::unlink(respfname);
    ::unlink(indexfname);
    ::unlink(cafname);
    ::unlink(cakeyfname);
    ::unlink(ocspfname);
    ::unlink(ocspkeyfname);

    return resp.get_status(cert, cacert);
}


WVTEST_MAIN("encoding request")
{
    WvRSAKey rsakey(DEFAULT_KEYLEN);
    WvString certreq
	= WvX509Mgr::certreq("cn=test.signed.com,dc=signed,dc=com", rsakey);

    WvX509Mgr cacert("CN=test.foo.com,DC=foo,DC=com", DEFAULT_KEYLEN, true);
    // cacert.set_ext_key_usage("OCSP Signing");
    // cacert.signcert(cacert); // resign self

    WvX509Mgr cert; // only need wvx509mgr to test bogus signer problem
    WvString certpem = cacert.signreq(certreq);
    cert.decode(WvX509Mgr::CertPEM, certpem);
    cert.set_rsa(&rsakey);

    // this test breaks after 2020. oh well.
    static const char *EXPDATE = "202010194703Z"; //dec 10, 2049
    static const char *REVDATE = "071211195254Z"; //dec 11 2007

    // unknown, because the cert's serial not in CRL list
    WVPASSEQ(test_ocsp_req(cert, cacert, cacert, ""), WvOCSPResp::Unknown);
    // revoked
    WVPASSEQ(test_ocsp_req(cert, cacert, cacert,
                           WvString("R\t%s\t%s\t%s\tunknown\t%s\n",
                                    EXPDATE, REVDATE, cert.get_serial(true),
                                    cert.get_subject())), 
             WvOCSPResp::Revoked);
    // good
    WVPASSEQ(test_ocsp_req(cert, cacert, cacert,
                           WvString("V\t%s\t%s\t%s\tunknown\t%s\n",
                                    EXPDATE, REVDATE, cert.get_serial(true),
                                    cert.get_subject())), 
             WvOCSPResp::Good);
}
