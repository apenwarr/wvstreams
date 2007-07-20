/*
 * Worldvisions Tunnel Vision Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * RSA cryptography abstractions.
 */
#include <assert.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include "wvsslhacks.h"
#include "wvrsa.h"
#include "wvhex.h"
#include "wvfileutils.h"

/***** WvRSAKey *****/

WvRSAKey::WvRSAKey()
    : debug("RSA", WvLog::Debug5)
{
    rsa = NULL;
}


WvRSAKey::WvRSAKey(const WvRSAKey &k)
    : debug("RSA", WvLog::Debug5)
{
    priv = k.priv;

    if (!priv)
        rsa = RSAPublicKey_dup(k.rsa);
    else
        rsa = RSAPrivateKey_dup(k.rsa);
}


WvRSAKey::WvRSAKey(struct rsa_st *_rsa, bool _priv)
    : debug("RSA", WvLog::Debug5)
{        
    if (_rsa == NULL)
    {
        rsa = NULL;
        debug("Initializing with a NULL key.. are you insane?\n");
	return;
    }

    rsa = _rsa;
    priv = _priv;
}


WvRSAKey::WvRSAKey(WvStringParm keystr, bool _priv)
    : debug("RSA", WvLog::Debug5)
{
    rsa = NULL;

    if (_priv)
        decode(RsaHex, keystr);
    else
        decode(RsaPubHex, keystr);

    priv = _priv;
}


WvRSAKey::WvRSAKey(int bits)
    : debug("RSA", WvLog::Debug5)
{
    rsa = RSA_generate_key(bits, 0x10001, NULL, NULL);
    priv = true;
}


WvRSAKey::~WvRSAKey()
{
    if (rsa)
        RSA_free(rsa);
}


bool WvRSAKey::isok() const
{
    return rsa && (!priv || RSA_check_key(rsa) == 1);
}


WvString WvRSAKey::encode(const DumpMode mode)
{
    WvString nil;
    WvDynBuf retval;
    encode(mode, retval);
    return retval.getstr();
}


void WvRSAKey::encode(const DumpMode mode, WvBuf &buf)
{
    if (!rsa)
    {
        debug(WvLog::Warning, "Tried to encode RSA key, but RSA key is "
              "blank!\n");
        return;
    }

    if (mode == RsaHex || mode == RsaPubHex)
    {
        WvDynBuf keybuf;

        if (mode == RsaHex && priv)
        {
            size_t size = i2d_RSAPrivateKey(rsa, NULL);
            unsigned char *key = keybuf.alloc(size);
            size_t newsize = i2d_RSAPrivateKey(rsa, & key);
            assert(size == newsize);
        }
        else
        {
            size_t size = i2d_RSAPublicKey(rsa, NULL);
            unsigned char *key = keybuf.alloc(size);
            size_t newsize = i2d_RSAPublicKey(rsa, & key);
            assert(size == newsize);
        }

        buf.putstr(WvString(WvHexEncoder().strflushbuf(keybuf, true)));
    }
    else
    {
        BIO *bufbio = BIO_new(BIO_s_mem());
        BUF_MEM *bm;
        const EVP_CIPHER *enc = EVP_get_cipherbyname("rsa");
    
        if (mode == RsaPEM)
            PEM_write_bio_RSAPrivateKey(bufbio, rsa, enc,
                                        NULL, 0, NULL, NULL);
        else if (mode == RsaPubPEM)
            PEM_write_bio_RSAPublicKey(bufbio, rsa);
        else
            debug(WvLog::Warning, "Should never happen: tried to encode RSA "
                  "key with unsupported mode.");

        BIO_get_mem_ptr(bufbio, &bm);
        buf.put(bm->data, bm->length);
        BIO_free(bufbio);
    }
}


void WvRSAKey::decode(const DumpMode mode, WvStringParm encoded)
{
    if (!encoded)
	return;
    
    WvDynBuf buf;
    buf.putstr(encoded);
    decode(mode, buf);
}


void WvRSAKey::decode(const DumpMode mode, WvBuf &encoded)
{
    debug("Decoding RSA key.\n");

    if (rsa)
    {
        debug("Replacing already existent RSA key.\n");
        RSA_free(rsa);
        rsa = NULL;
    }
    priv = false;

    // we handle hexified keys a bit differently, since
    // OpenSSL has no built-in support for them...
    if (mode == RsaHex || mode == RsaPubHex)
    {
        // unhexify the supplied key
        WvDynBuf keybuf;
        if (!WvHexDecoder().flush(encoded, keybuf, true) || 
            keybuf.used() == 0)
        {
            debug("Couldn't unhexify RSA key.\n");
            return;
        }
    
        size_t keylen = keybuf.used();
        const unsigned char *key = keybuf.get(keylen);
    
        // create the RSA struct
        if (mode == RsaHex)
        {
            rsa = wv_d2i_RSAPrivateKey(NULL, &key, keylen);
            priv = true;
        }
        else
            rsa = wv_d2i_RSAPublicKey(NULL, &key, keylen);

        return;
    }
    else
    {

        BIO *membuf = BIO_new(BIO_s_mem());
        BIO_write(membuf, encoded.get(encoded.used()), encoded.used());

        if (mode == RsaPEM)
        {
            rsa = PEM_read_bio_RSAPrivateKey(membuf, NULL, NULL, NULL);
            priv = true;
        }
        else if (mode == RsaPubPEM)
            rsa = PEM_read_bio_RSAPublicKey(membuf, NULL, NULL, NULL);
        else 
            debug(WvLog::Warning, "Should never happen: tried to encode RSA "
                  "key with unsupported mode.");

        BIO_free_all(membuf);
    }
}


/***** WvRSAEncoder *****/

WvRSAEncoder::WvRSAEncoder(Mode _mode, const WvRSAKey & _key) :
    mode(_mode), key(_key)
{
    if (key.isok() && key.rsa != NULL)
        rsasize = RSA_size(key.rsa);
    else
        rsasize = 0; // BAD KEY! (should assert but would break compatibility)
}


WvRSAEncoder::~WvRSAEncoder()
{
}


bool WvRSAEncoder::_reset()
{
    return true;
}


bool WvRSAEncoder::_encode(WvBuf &in, WvBuf &out, bool flush)
{
    if (rsasize == 0)
    {
        // IGNORE BAD KEY!
        in.zap();
        return false;
    }
        
    bool success = true;
    switch (mode)
    {
        case Encrypt:
        case SignEncrypt:
        {
            // reserve space for PKCS1_PADDING
            const size_t maxchunklen = rsasize - 12;
            size_t chunklen;
            while ((chunklen = in.used()) != 0)
            {
                if (chunklen >= maxchunklen)
                    chunklen = maxchunklen;
                else if (! flush)
                    break;

                // encrypt a chunk
                const unsigned char *data = in.get(chunklen);
                unsigned char *crypt = out.alloc(rsasize);
                size_t cryptlen = (mode == Encrypt) ?
                    RSA_public_encrypt(chunklen,
                    const_cast<unsigned char*>(data), crypt,
                    key.rsa, RSA_PKCS1_PADDING) :
                    RSA_private_encrypt(chunklen,
                    const_cast<unsigned char*>(data), crypt,
                    key.rsa, RSA_PKCS1_PADDING);
                if (cryptlen != rsasize)
                {
                    out.unalloc(rsasize);
                    success = false;
                }
            }
            break;
        }
        case Decrypt:
        case SignDecrypt:
        {
            const size_t chunklen = rsasize;
            while (in.used() >= chunklen)
            {
                // decrypt a chunk
                const unsigned char *crypt = in.get(chunklen);
                unsigned char *data = out.alloc(rsasize);
                int cryptlen = (mode == Decrypt) ?
                    RSA_private_decrypt(chunklen,
                    const_cast<unsigned char*>(crypt), data,
                    key.rsa, RSA_PKCS1_PADDING) :
                    RSA_public_decrypt(chunklen,
                    const_cast<unsigned char*>(crypt), data,
                    key.rsa, RSA_PKCS1_PADDING);
                if (cryptlen == -1)
                {
                    out.unalloc(rsasize);
                    success = false;
                }
                else
                    out.unalloc(rsasize - cryptlen);
            }
            // flush does not make sense for us here
            if (flush && in.used() != 0)
                success = false;
            break;
        }
    }
    return success;
}


/***** WvRSAStream *****/

WvRSAStream::WvRSAStream(WvStream *_cloned,
    const WvRSAKey &_my_key, const WvRSAKey &_their_key,
    WvRSAEncoder::Mode readmode, WvRSAEncoder::Mode writemode) :
    WvEncoderStream(_cloned)
{
    readchain.append(new WvRSAEncoder(readmode, _my_key), true);
    writechain.append(new WvRSAEncoder(writemode, _their_key), true);
    if (_my_key.isok() && _my_key.rsa)
        min_readsize = RSA_size(_my_key.rsa);
}
