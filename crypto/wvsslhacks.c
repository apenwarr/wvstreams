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

RSA *wv_d2i_RSAPublicKey(RSA **a, const unsigned char **pp, long length)
{
    return d2i_RSAPublicKey(a, pp, length);
}


RSA *wv_d2i_RSAPrivateKey(RSA **a, const unsigned char **pp, long length)
{
    return d2i_RSAPrivateKey(a, pp, length);
}


X509 *wv_d2i_X509(X509 **a, const unsigned char **pp, long length)
{
    return d2i_X509(a, pp, length);
}
