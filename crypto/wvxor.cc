/*
 * Worldvisions Tunnel Vision Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * XOR cryptography abstractions.
 * Could use this to implement short one time pads.
 */
#include "wvxor.h"

/***** WvXOREncoder *****/

WvXOREncoder::WvXOREncoder(const void *_key, size_t _keylen) :
    keylen(_keylen), keyoff(0)
{
    key = new unsigned char[keylen];
    memcpy(key, _key, keylen);
}


WvXOREncoder::~WvXOREncoder()
{
    delete[] key;
}


bool WvXOREncoder::_encode(WvBuffer &inbuf, WvBuffer &outbuf, bool flush)
{
    size_t len;
    while ((len = inbuf.usedopt()) != 0)
    {
        const unsigned char *data = inbuf.get(len);
        unsigned char *out = outbuf.alloc(len);

        // FIXME: this loop is SLOW! (believe it or not)
        while (len-- > 0)
        {
            *out++ = (*data++) ^ key[keyoff++];
            keyoff %= keylen;
        }
    }
    return true;
}

/***** WvXORStream *****/

WvXORStream::WvXORStream(WvStream *_cloned,
    const void *_key, size_t _keysize) :
    WvEncoderStream(_cloned)
{
    readchain.append(new WvXOREncoder(_key, _keysize), true);
    writechain.append(new WvXOREncoder(_key, _keysize), true);
}
