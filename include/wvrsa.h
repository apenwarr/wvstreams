/*
 * Worldvisions Tunnel Vision Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * RSA cryptography abstractions.
 */
#ifndef __WVRSA_H
#define __WVRSA_H

#include "wverror.h"
#include "wvencoder.h"
#include "wvencoderstream.h"

/**
 * An RSA public key (or public/private key pair) that can be used for
 * encryption.  Knows how to encode/decode itself into a stream of hex
 * digits for easy transport.
 */
struct rsa_st;
class WvRSAKey : public WvError
{
    int errnum;
    WvString pub, prv;

    void init(WvStringParm keystr, bool priv);
    static WvString hexifypub(struct rsa_st *rsa);
    static WvString hexifyprv(struct rsa_st *rsa);

public:
    struct rsa_st *rsa;

    WvRSAKey(const WvRSAKey &k);
    WvRSAKey(struct rsa_st *_rsa, bool priv); // note: takes ownership
    WvRSAKey(WvStringParm keystr, bool priv);
    WvRSAKey(int bits);
    
    ~WvRSAKey();
    
    WvString private_str() const
        { return prv; }
    WvString public_str() const
        { return pub; }
};


/**
 * An encoder implementing RSA public/private key encryption.
 * This is really slow, so should only be used to exchange information
 * about a faster symmetric key (like Blowfish).
 *
 * RSA needs to know the public key from the remote end (to encrypt data) and
 * the private key on this end (to decrypt data).
 */
class WvRSAEncoder : public WvEncoder
{
public:
    enum Mode {
        Encrypt, // encrypt with public key
        Decrypt, // decrypt with private key
        SignEncrypt, // encrypt signature with public key 
        SignDecrypt  // decrypt signature with private key
    };

    /**
     * Creates a new RSA encoder / decoder.
     *   _mode : the encryption mode
     *   _key  : the public key for encryption if _encrypt == true,
     *           otherwise the private key for decryption
     */
    WvRSAEncoder(Mode _mode, const WvRSAKey &_key);
    virtual ~WvRSAEncoder();

protected:
    virtual bool _encode(WvBuffer &in, WvBuffer &out, bool flush);
    virtual bool _reset(); // supported

private:
    Mode mode;
    WvRSAKey key;
    size_t rsasize;
};


/**
 * A crypto stream implementing RSA public/private key encryption.
 * See WvRSAEncoder for details.
 *
 * By default, written data is "encrypted", read data is "decrypted".
 */
class WvRSAStream : public WvEncoderStream
{
public:
    WvRSAStream(WvStream *_cloned,
        const WvRSAKey &_my_key, const WvRSAKey &_their_key, 
        WvRSAEncoder::Mode readmode = WvRSAEncoder::Decrypt,
        WvRSAEncoder::Mode writemode = WvRSAEncoder::Encrypt);
    virtual ~WvRSAStream() { }
};


#endif // __WVRSA_H
