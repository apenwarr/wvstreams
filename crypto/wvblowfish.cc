/*
 * Worldvisions Tunnel Vision Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Blowfish cryptography abstractions.
 */
#include "wvblowfish.h"
#include <assert.h>
#include <rand.h>
#include <blowfish.h>

/***** WvBlowfishEncoder ****/

WvBlowfishEncoder::WvBlowfishEncoder(Mode _mode, bool _encrypt,
    const void *_key, size_t _keysize) :
    mode(_mode), encrypt(_encrypt), key(NULL)
{
    setkey(_key, _keysize);
}


WvBlowfishEncoder::~WvBlowfishEncoder()
{
    delete key;
}


void WvBlowfishEncoder::setkey(const void *_key, size_t _keysize)
{
    delete key;
    key = new BF_KEY;
    keysize = _keysize;
    BF_set_key(key, _keysize, (unsigned char *)_key);
    memset(ivec, 0, sizeof(ivec));
    ivecoff = 0;
}


bool WvBlowfishEncoder::encode(WvBuffer &in, WvBuffer &out, bool flush)
{
    size_t len = in.used();
    bool success = true;
    if (mode == ECB)
    {
        size_t remainder = len & 7;
        len -= remainder;
        if (remainder != 0 && flush)
        {
            if (encrypt)
            {
                // if flushing on encryption, add some randomized padding
                size_t padlen = 8 - remainder;
                unsigned char *pad = in.alloc(padlen);
                RAND_pseudo_bytes(pad, padlen);
                len += 8;
            }
            else // nothing we can do here, flushing does not make sense!
                success = false;
        }
    }
    if (len == 0) return success;
    
    unsigned char *data = in.get(len);
    unsigned char *crypt = out.alloc(len);
    
    switch (mode)
    {
        case ECB:
            // ECB works 64bits at a time
            while (len >= 8)
            {
                BF_ecb_encrypt(data, crypt, key,
                    encrypt ? BF_ENCRYPT : BF_DECRYPT);
                len -= 8;
                data += 8;
                crypt += 8;
            }
            break;

        case CFB:
            // CFB simulates a stream
            BF_cfb64_encrypt(data, crypt, len, key, ivec,
                &ivecoff, encrypt ? BF_ENCRYPT : BF_DECRYPT);
            break;
    }
    return success;
}


/***** WvBlowfishStream *****/

WvBlowfishStream::WvBlowfishStream(WvStream *_cloned,
    const void *_key, size_t _keysize) :
    WvEncoderStream(_cloned)
{
    readchain.append(new WvBlowfishEncoder(WvBlowfishEncoder::CFB,
        false /*encrypt*/, _key, _keysize), true);
    writechain.append(new WvBlowfishEncoder(WvBlowfishEncoder::CFB,
        true /*encrypt*/, _key, _keysize), true);
}
