/*
 * Worldvisions Weaver Software:
 * Copyright (C) 2003 Net Integration Technologies, Inc.
 *
 * Diffie-Hellman shared secret creation.
 */

#ifndef __WVOAKLEY_H
#define __WVOAKLEY_H

#include "wvstream.h"
#include "wvdiffiehellman.h"

class WvOakleyAuth
{
public:
    WvOakleyAuth(int group);
    short public_len();
    short get_public_key(WvBuf &outbuf, short len);
    void create_secret(WvBuf &inbuf, short len);

private:
    WvDiffieHellman *dh;
    WvDynBuf dh_secret;
    short pub_len;
    short secret_len;
};

#endif /* __WVOAKLEY_H */
