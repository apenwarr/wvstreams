/*
 * Worldvisions Tunnel Vision Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */

/** \file
 * Blowfish cryptography abstractions.
 */
#ifndef __WVBLOWFISH_H
#define __WVBLOWFISH_H

#include "wvencoder.h"
#include "wvencoderstream.h"

struct bf_key_st;

/**
 * An encoder implementing the Blowfish encryption method.
 * <p>
 * Supports reset().
 * </p>
 */
class WvBlowfishEncoder : public WvEncoder
{
public:
    enum Mode {
        ECBEncrypt, /*!< Encrypt using ECB mode (avoid) */
        ECBDecrypt, /*!< Decrypt using ECB mode (avoid) */
        CFBEncrypt, /*!< Encrypt using CFB mode (simulates a stream) */
        CFBDecrypt  /*!< Decrypt using CFB mode (simulates a stream) */
    };

    /**
     * Creates a new Blowfish cipher encoder.
     *
     * @param mode the encryption mode
     * @param key the initial key
     * @param keysize the initial key size in bytes
     */
    WvBlowfishEncoder(Mode mode, const void *key, size_t keysize);
    virtual ~WvBlowfishEncoder();

    /**
     * Sets the current Blowfish key and resets the initialization
     * vector to all nulls.
     *
     * @param key the new key
     * @param keysize the key size in bytes
     */
    void setkey(const void *key, size_t keysize);
    
    /**
     * Sets the current Blowfish initialization vector.
     *
     * @param iv the new IV must be 8 bytes
     */
    void setiv(const void *iv);

protected:
    virtual bool _encode(WvBuffer &in, WvBuffer &out, bool flush);
    virtual bool _reset(); // supported: restores most recently set
        // key and initialization vector

private:
    Mode mode;
    size_t keysize;
    unsigned char *key;
    struct bf_key_st *bfkey;
    unsigned char ivec[8]; // initialization vector
    int ivecoff; // current offset into initvec

    void preparekey();
};


/**
 * A crypto stream implementing Blowfish encryption.
 * <p>
 * By default, written data is encrypted using
 * WvBlowfishEncoder::CFBEncrypt, read data is decrypted using
 * WvBlowfishEncoder::CFBDecrypt.
 * </p>
 * @see WvBlowfishEncoder
 */
class WvBlowfishStream : public WvEncoderStream
{
public:
    WvBlowfishStream(WvStream *_cloned,
        const void *key, size_t _keysize,
        WvBlowfishEncoder::Mode readmode = WvBlowfishEncoder::CFBDecrypt,
        WvBlowfishEncoder::Mode writemode = WvBlowfishEncoder::CFBEncrypt);
    virtual ~WvBlowfishStream() { }
};

#endif // __WVBLOWFISH_H
