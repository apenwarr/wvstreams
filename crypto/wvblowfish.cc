/*
 * Worldvisions Tunnel Vision Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Blowfish cryptography abstractions.
 */
#include "wvblowfish.h"
#include <assert.h>
#include <openssl/rand.h>
#include <openssl/blowfish.h>

/***** WvBlowfishEncoder ****/

WvBlowfishEncoder::WvBlowfishEncoder(Mode _mode,
    const void *_key, size_t _keysize) :
    mode(_mode), key(NULL), bfkey(NULL)
{
    setkey(_key, _keysize);
}


WvBlowfishEncoder::~WvBlowfishEncoder()
{
    delete[] key;
    delete bfkey;
}


bool WvBlowfishEncoder::_reset()
{
    preparekey();
    return true;
}


void WvBlowfishEncoder::setkey(const void *_key, size_t _keysize)
{
    delete[] key;
    keysize = _keysize;
    key = new unsigned char[keysize];
    memcpy(key, _key, keysize);
    preparekey();
}


void WvBlowfishEncoder::preparekey()
{
    delete bfkey;
    bfkey = new BF_KEY;
    BF_set_key(bfkey, keysize, key);
    memset(ivec, 0, sizeof(ivec));
    ivecoff = 0;
}


bool WvBlowfishEncoder::_encode(WvBuffer &in, WvBuffer &out, bool flush)
{
    size_t len = in.used();
    bool success = true;
    switch (mode) {
        case ECBEncrypt:
        case ECBDecrypt:
        {
            size_t remainder = len & 7;
            len -= remainder;
            if (remainder != 0 && flush)
            {
                if (mode == ECBEncrypt)
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

        default:
            break;
    }
    if (len == 0) return success;
    
    const unsigned char *data = in.get(len);
    unsigned char *crypt = out.alloc(len);
    
    switch (mode)
    {
        case ECBEncrypt:
        case ECBDecrypt:
            // ECB works 64bits at a time
            while (len >= 8)
            {
                BF_ecb_encrypt(data, crypt, bfkey,
                    mode == ECBEncrypt ? BF_ENCRYPT : BF_DECRYPT);
                len -= 8;
                data += 8;
                crypt += 8;
            }
            break;

        case CFBEncrypt:
        case CFBDecrypt:
            // CFB simulates a stream
            BF_cfb64_encrypt(data, crypt, len, bfkey, ivec, &ivecoff,
                mode == CFBEncrypt ? BF_ENCRYPT : BF_DECRYPT);
            break;
    }
    return success;
}


/***** WvBlowfishStream *****/

WvBlowfishStream::WvBlowfishStream(WvStream *_cloned,
    const void *_key, size_t _keysize,
    WvBlowfishEncoder::Mode readmode, WvBlowfishEncoder::Mode writemode) :
    WvEncoderStream(_cloned)
{
    readchain.append(new WvBlowfishEncoder(readmode,
        _key, _keysize), true);
    writechain.append(new WvBlowfishEncoder(writemode,
        _key, _keysize), true);
}
