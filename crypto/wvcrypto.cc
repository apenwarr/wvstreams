/*
 * Worldvisions Tunnel Vision Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Miscellaneous cryptography primitives.
 */
#include "wvcrypto.h"

/***** WvRandomStream *****/

WvRandomStream::WvRandomStream() :
    WvFile("/dev/urandom", O_RDONLY)
{
}
