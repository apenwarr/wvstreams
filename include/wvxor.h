/*
 * Worldvisions Tunnel Vision Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * XOR cryptography abstractions.
 * Could use this to implement short one time pads.
 */
#ifndef __WVXOR_H
#define __WVXOR_H

#include "wvencoder.h"
#include "wvencoderstream.h"

/**
 * An encoder implementing simple XOR encryption.
 * Mainly useful for testing.
 */
class WvXOREncoder : public WvEncoder
{
public:
    /**
     * Creates a new XOR encoder / decoder.
     *   _key    : the key
     *   _keylen : the length of the key in bytes
     */
    WvXOREncoder(const void *_key, size_t _keylen);
    virtual ~WvXOREncoder();
    
protected:
    bool _encode(WvBuffer &in, WvBuffer &out, bool flush);

private:
    unsigned char *key;
    size_t keylen;
    int keyoff;
};


/**
 * A crypto stream implementing XOR encryption.
 * See WvXOREncoder for details.
 */
class WvXORStream : public WvEncoderStream
{
public:
    WvXORStream(WvStream *_cloned, const void *key, size_t _keysize);
    virtual ~WvXORStream() { }
};

#endif /// __WVXOR_H
