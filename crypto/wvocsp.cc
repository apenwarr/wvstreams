#include "wvocsp.h"


static const int OCSP_MAX_VALIDITY_PERIOD = (5 * 60); // 5 min: openssl default


WvOCSPReq::WvOCSPReq(const WvX509 &cert, const WvX509 &issuer)
{
    req = OCSP_REQUEST_new();

    // FIXME: follow return values of all these things

    id = OCSP_cert_to_id(NULL, cert.cert, issuer.cert);
    OCSP_request_add0_id(req, id);
}


WvOCSPReq::~WvOCSPReq()
{
    OCSP_REQUEST_free(req);
    //OCSP_CERTID_free(id);
}


void WvOCSPReq::encode(WvBuf &buf)
{
    BIO *bufbio = BIO_new(BIO_s_mem());
    BUF_MEM *bm;

    if (i2d_OCSP_REQUEST_bio(bufbio, req) <= 0)
    {
        // .. FIXME: handle failure via an error code
    }

    BIO_get_mem_ptr(bufbio, &bm);
    buf.put(bm->data, bm->length);
    BIO_free(bufbio);
}


WvOCSPResp::WvOCSPResp(const WvX509 &_issuer, const WvX509 &_responder) :
    issuer(_issuer),
    responder(_responder)
{
}


void WvOCSPResp::decode(WvBuf &encoded)
{
    BIO *membuf = BIO_new(BIO_s_mem());
    BIO_write(membuf, encoded.get(encoded.used()), encoded.used());

    resp = d2i_OCSP_RESPONSE_bio(membuf, NULL);

    BIO_free_all(membuf);
}


WvOCSPResp::Status WvOCSPResp::get_status(const WvOCSPReq &req) const
{
    X509_STORE *store = X509_STORE_new();
    if (!resp)
        return ERROR;

    wvcon->print("Getting response status.\n");
    int i = OCSP_response_status(resp);
    wvcon->print("response status is %s.\n", i);
    if (i != OCSP_RESPONSE_STATUS_SUCCESSFUL)
        return ERROR;

    wvcon->print("Getting basic response.\n");
    OCSP_BASICRESP * bs = OCSP_response_get1_basic(resp);
    if (!bs)
        return ERROR;

    if ((i = OCSP_check_nonce(req.req, bs)) <= 0)
    {
        if (i == -1)
            wvcon->print("WARNING: no nonce in response\n");
        else
        {
            wvcon->print("Nonce Verify error\n");
            return ERROR;
        }
    }

    wvcon->print("Nonce verified ok?\n");

    STACK_OF(X509) *verify_other = sk_X509_new_null();
    sk_X509_push(verify_other, responder.cert);

    i = OCSP_basic_verify(bs, verify_other, store, OCSP_TRUSTOTHER);

    sk_free(verify_other);

    X509_STORE_free(store);

    if (i <= 0)
    {
        wvcon->print("OCSP Verify Error: %s\n", wvssl_errstr());
        return ERROR;
    }

    int status, reason;

    ASN1_GENERALIZEDTIME *rev, *thisupd, *nextupd;

    if(!OCSP_resp_find_status(bs, req.id, &status, &reason,
                              &rev, &thisupd, &nextupd))
    {
        wvcon->print("OCSP Find Status Error: %s\n", wvssl_errstr());
        return ERROR;
    }

    if (!OCSP_check_validity(thisupd, nextupd, OCSP_MAX_VALIDITY_PERIOD, -1))
    {
        wvcon->print("Error checking for OCSP validity: %s\n", wvssl_errstr());
        return ERROR;
    }

    if (status == V_OCSP_CERTSTATUS_GOOD)
        return GOOD;
    else if (status == V_OCSP_CERTSTATUS_REVOKED)
        return REVOKED;
    
    return UNKNOWN;
}
