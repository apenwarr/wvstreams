#include "wvocsp.h"
#include "wvsslhacks.h"

static const int OCSP_MAX_VALIDITY_PERIOD = (5 * 60); // 5 min: openssl default


WvOCSPReq::WvOCSPReq(const WvX509 &cert, const WvX509 &issuer)
{
    wvssl_init();

    req = OCSP_REQUEST_new();
    assert(req);

    if (cert.isok() && issuer.isok())
    {
        id = OCSP_cert_to_id(NULL, cert.cert, issuer.cert);
        OCSP_request_add0_id(req, id);
    }
}


WvOCSPReq::~WvOCSPReq()
{
    if (req)
        OCSP_REQUEST_free(req);

    wvssl_free();
}


void WvOCSPReq::encode(WvBuf &buf)
{
    BIO *bufbio = BIO_new(BIO_s_mem());
    assert(bufbio);
    BUF_MEM *bm;

    // there is no reason why the following should fail, except for OOM
    assert(wv_i2d_OCSP_REQUEST_bio(bufbio, req) > 0);

    BIO_get_mem_ptr(bufbio, &bm);
    buf.put(bm->data, bm->length);
    BIO_free(bufbio);
}


WvOCSPResp::WvOCSPResp() :
    resp(NULL),
    bs(NULL),
    log("OCSP Response", WvLog::Debug5)
{
    wvssl_init();
}


WvOCSPResp::~WvOCSPResp()
{
    if (bs)
        OCSP_BASICRESP_free(bs);
        
    if (resp)
        OCSP_RESPONSE_free(resp);

    wvssl_free();
}


void WvOCSPResp::decode(WvBuf &encoded)
{
    BIO *membuf = BIO_new(BIO_s_mem());
    BIO_write(membuf, encoded.get(encoded.used()), encoded.used());

    resp = d2i_OCSP_RESPONSE_bio(membuf, NULL);
    
    if (resp)
        bs = OCSP_response_get1_basic(resp);
    else
        log("Failed to decode response.\n");

    BIO_free_all(membuf);
}


bool WvOCSPResp::isok() const
{
    if (!resp)
        return false;

    int i = OCSP_response_status(resp);
    if (i != OCSP_RESPONSE_STATUS_SUCCESSFUL)
    {
        log("Status not successful: %s\n", wvssl_errstr());
        return false;
    }

    return true;
}


bool WvOCSPResp::check_nonce(const WvOCSPReq &req) const
{
    if (!bs)
        return false;

    int i;
    if ((i = OCSP_check_nonce(req.req, bs)) <= 0)
    {
        if (i == -1)
            log("No nonce in response\n");
        else
            log("Nonce verify error\n");
        
        return false;
    }

    return true;
}


bool WvOCSPResp::signedbycert(const WvX509 &cert) const
{
    STACK_OF(X509) *sk = sk_X509_new_null();
    sk_X509_push(sk, cert.cert);
    int i = OCSP_basic_verify(bs, sk, NULL, OCSP_NOVERIFY);
    sk_X509_free(sk);

    if(i > 0)
        return true;

    return false;    
}


WvX509 WvOCSPResp::get_signing_cert() const
{
    const STACK_OF(X509) *certs = OCSP_resp_get0_certs(bs);
    if (!bs || !sk_X509_num(certs))
        return WvX509();

    X509 *signer = NULL;
    if (OCSP_resp_get0_signer(bs, &signer, NULL) == 1) {
        return WvX509(X509_dup(signer));
    }

    return WvX509();
}


WvOCSPResp::Status WvOCSPResp::get_status(const WvX509 &cert, 
                                          const WvX509 &issuer) const
{
    if (!isok())
        return Error;

    if (!cert.isok() && !issuer.isok())
        return Error;

    int status, reason;
    ASN1_GENERALIZEDTIME *rev, *thisupd, *nextupd;

    OCSP_CERTID *id = OCSP_cert_to_id(NULL, cert.cert, issuer.cert);
    assert(id); // only fails in case of OOM

    if(!OCSP_resp_find_status(bs, id, &status, &reason,
                              &rev, &thisupd, &nextupd))
    {
        log("OCSP Find Status Error: %s\n", wvssl_errstr());
        OCSP_CERTID_free(id);
        return Error;
    }
    OCSP_CERTID_free(id);

    if (!OCSP_check_validity(thisupd, nextupd, OCSP_MAX_VALIDITY_PERIOD, -1))
    {
        log("Error checking for OCSP validity: %s\n", wvssl_errstr());
        return Error;
    }

    if (status == V_OCSP_CERTSTATUS_GOOD)
        return Good;
    else if (status == V_OCSP_CERTSTATUS_REVOKED)
        return Revoked;

    log("OCSP cert status is %s, marking as 'Unknown'.\n", 
        OCSP_cert_status_str(status));
    
    return Unknown;
}

WvString WvOCSPResp::status_str(WvOCSPResp::Status status)
{
    if (status == Good)
        return "good";
    else if (status == Error)
        return "error";
    else if (status == Revoked)
        return "revoked";

    return "unknown";
}
