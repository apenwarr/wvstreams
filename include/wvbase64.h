/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Base64 encoder and decoder implementations.
 */
#ifndef __BASE64_H
#define __BASE64_H

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

    // on flush, outputs any needed pad characters
    virtual bool encode(WvBuffer &in, WvBuffer &out, bool flush);
};


class WvBase64Decoder : public WvEncoder
{
    enum State {
        ATBIT0, ATBIT2, ATBIT4, ATBIT6, PAD, ERROR
    };
    State state;
    unsigned int bits; // remaining bits shifted left 6 bits
public:
    WvBase64Decoder();
    virtual ~WvBase64Decoder() { }

    // returns false if an error was detected in the input
    virtual bool isok() const;
    virtual bool encode(WvBuffer &in, WvBuffer &out, bool flush);
};

#endif // __BASE64_H
