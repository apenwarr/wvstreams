/*
 * Worldvisions Tunnel Vision Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Miscellaneous cryptography primitives.
 */
#ifndef __WVCRYPTO_H
#define __WVCRYPTO_H

#include "wvfile.h"


/** A very simple stream that returns randomness from /dev/urandom */
class WvRandomStream : public WvFile
{
public:
    WvRandomStream();
};

#endif // __WVCRYPTO_H
