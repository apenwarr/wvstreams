/* -*- Mode: C++ -*-
 *
 * OCSP request and response abstractions.
 *
 * OCSP provides a quick way of checking whether a certificate is valid or 
 * not. For more information, see: http://en.wikipedia.org/wiki/OCSP
 *
 * For the sake of both ease of implementation and use, these classes only 
 * expose a simplified subset of OCSP functionality:
 *  - A nonce (unique identifier for the request) is required.
 *  - Both the request and response objects assume only one certificate is to 
 *    be validated.
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
    WvOCSPResp();
    virtual ~WvOCSPResp();

    void decode(WvBuf &buf);
  
    enum Status { ERROR, GOOD, REVOKED, UNKNOWN };
    Status get_status(const WvOCSPReq &req, const WvX509 &issuer, 
                      const WvX509 &responder) const;

private:
    OCSP_RESPONSE *resp;
    mutable WvLog log;
};

#endif // __WVOCSP_H
