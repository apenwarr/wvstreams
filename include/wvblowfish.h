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
        ECBEncrypt, // electronic code book mode (avoid!)
        ECBDecrypt,
        CFBEncrypt, // cipher feedback mode (simulates a stream)
        CFBDecrypt
    };

    /**
     * Creates a new Blowfish encoder / decoder.
     *   _mode    : the encryption mode
     *   _key     : the initial key
     *   _keysize : the initial key size in bytes
     */
    WvBlowfishEncoder(Mode _mode, const void *_key, size_t _keysize);
    virtual ~WvBlowfishEncoder();

    /**
     * Sets the current Blowfish key and resets the initialization
     * vector to all nulls.
     */
    void setkey(const void *_key, size_t _keysize);

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
 * See WvBlowfishEncoder for details.
 *
 * By default, written data is "cfbencrypted", read data is "cfbdecrypted".
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
