/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Base64 encoder and decoder implementations.
 */
#ifndef __WVBASE64_H
#define __WVBASE64_H

#include "wvencoder.h"

class WvBase64Encoder : public WvEncoder
{
    enum State {
        ATBIT0, ATBIT2, ATBIT4
    };
    State state;
    unsigned int bits; // remaining bits shifted left 8 bits
public:
    WvBase64Encoder();
    virtual ~WvBase64Encoder() { }

protected:
    // on flush, outputs any needed pad characters
    virtual bool _encode(WvBuffer &in, WvBuffer &out, bool flush);
};


class WvBase64Decoder : public WvEncoder
{
    enum State {
        ATBIT0, ATBIT2, ATBIT4, ATBIT6, PAD
    };
    State state;
    unsigned int bits; // remaining bits shifted left 6 bits
public:
    WvBase64Decoder();
    virtual ~WvBase64Decoder() { }

protected:
    virtual bool _encode(WvBuffer &in, WvBuffer &out, bool flush);
};

#endif // __WVBASE64_H
