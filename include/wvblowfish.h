/*
 * Worldvisions Tunnel Vision Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Blowfish cryptography abstractions.
 */
#ifndef __WVBLOWFISH_H
#define __WVBLOWFISH_H

#include "wvencoder.h"
#include "wvencoderstream.h"

/**
 * A Blowfish encoder.
 */
struct bf_key_st;
class WvBlowfishEncoder : public WvEncoder
{
public:
    enum Mode {
        ECB, // electronic code book mode (avoid!)
        CFB  // cipher feedback mode (simulates a stream)
    };

    /**
     * Creates a new Blowfish encoder / decoder.
     *   _mode    : the encryption mode
     *   _encrypt : if true, encrypts else decrypts
     *   _key     : the initial key data
     *   _keysize : the initial key size
     */
    WvBlowfishEncoder(Mode _mode, bool _encrypt,
        const void *_key, size_t _keysize);
    virtual ~WvBlowfishEncoder();

    /**
     * Sets the current Blowfish key and resets the initialization
     * vector to all nulls.
     */
    void setkey(const void *_key, size_t _keysize);

    virtual bool encode(WvBuffer &in, WvBuffer &out, bool flush);

private:
    Mode mode;
    bool encrypt;
    size_t keysize;
    struct bf_key_st *key;
    unsigned char ivec[8]; // initialization vector
    int ivecoff; // current offset into initvec
};


/**
 * A crypto stream implementing Blowfish CFB encryption.
 * See WvBlowfishEncoder for details.
 */
class WvBlowfishStream : public WvEncoderStream
{
public:
    WvBlowfishStream(WvStream *_cloned, const void *key, size_t _keysize);
    virtual ~WvBlowfishStream() { }
};

#endif // __WVBLOWFISH_H
