/*
 * Worldvisions Tunnel Vision Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Functions to make us compile with both newer and older versions of openssl.
 * 
 * THIS FILE MUST BE COMPILED AS STANDARD C, NOT C++!!!
 * 
 * See wvsslhacks.h.
 */
#include "wvsslhacks.h"

#include "openssl/ocsp.h"

RSA *wv_d2i_RSAPublicKey(RSA **a, const unsigned char **pp, long length)
{
    return d2i_RSAPublicKey(a, (void *)pp, length);
}


RSA *wv_d2i_RSAPrivateKey(RSA **a, const unsigned char **pp, long length)
{
    return d2i_RSAPrivateKey(a, (void *)pp, length);
}

DSA *wv_d2i_DSAPublicKey(DSA **a, const unsigned char **pp, long length)
{
    return d2i_DSAPublicKey(a, (void *)pp, length);
}


DSA *wv_d2i_DSAPrivateKey(DSA **a, const unsigned char **pp, long length)
{
    return d2i_DSAPrivateKey(a, (void *)pp, length);
}


X509_REQ *wv_d2i_X509_REQ(X509_REQ **a, const unsigned char **pp, long length)
{
    return d2i_X509_REQ(a, (void *)pp, length);
}

X509 *wv_d2i_X509(X509 **a, unsigned char **pp, long length)
{
    return d2i_X509(a, (void *)pp, length);
}

int wv_i2d_OCSP_REQUEST_bio(BIO *bio, OCSP_REQUEST *req)
{
    return i2d_OCSP_REQUEST_bio(bio, req);
}
