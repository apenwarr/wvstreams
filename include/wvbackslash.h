/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 * 
 * Performs C-style backslash escaping and unescaping of strings.
 */
#ifndef __WVBACKSLASH_H
#define __WVBACKSLASH_H

#include "wvencoder.h"

class WvBackslashEncoder : public WvEncoder
{
public:
    WvBackslashEncoder();
    virtual ~WvBackslashEncoder() { }

protected:
    virtual bool _encode(WvBuffer &inbuf, WvBuffer &outbuf, bool flush);
    virtual bool _reset();
};


class WvBackslashDecoder : public WvEncoder
{
    enum State
        { Initial, Escape, Hex1, Hex2, Octal1, Octal2, Octal3 };
    State state;
    WvInPlaceBuffer tmpbuf;
    int value;

public:
    WvBackslashDecoder();
    virtual ~WvBackslashDecoder() { }

protected:
    virtual bool _encode(WvBuffer &inbuf, WvBuffer &outbuf, bool flush);
    virtual bool _reset();

private:
    bool flushtmpbuf(WvBuffer &outbuf);
};

#endif // __WVBACKSLASH_H
