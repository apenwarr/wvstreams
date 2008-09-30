/* -*- Mode: C++ -*-
 *
 * OCSP request and response abstractions.
 *
 * OCSP provides a quick way of checking whether a certificate is valid or not. 
 * For more information, see: http://en.wikipedia.org/wiki/OCSP
 *
 * For the sake of simplicity, these classes only expose a simplified subset of
 * OCSP functionality:
 *  - A nonce is required.
 *  - Both the request and response objects assume only one certificate is to 
 *    be validated
 *
 */ 
#ifndef __WVOCSP_H
#define __WVOCSP_H
#include "wvx509.h"

#include <openssl/ocsp.h>

class WvOCSPReq
{
public:
    WvOCSPReq(const WvX509 &cert, const WvX509 &issuer);
    virtual ~WvOCSPReq();

    void encode(WvBuf &buf);

private:
    friend class WvOCSPResp;
    OCSP_CERTID *id;
    OCSP_REQUEST *req;
};


class WvOCSPResp
{
public:
    WvOCSPResp(const WvX509 &_issuer, const WvX509 &responder);
    
    void decode(WvBuf &buf);
  
    enum Status { ERROR, GOOD, REVOKED, UNKNOWN };
    Status get_status(const WvOCSPReq &req) const;

    OCSP_RESPONSE *resp;

private:
    WvX509 issuer;
    mutable WvX509 responder;

};

#endif // __WVOCSP_H
