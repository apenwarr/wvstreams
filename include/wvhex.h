/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Hex encoder and hex decoder.
 * The hex decoder is not sensitive to whitespace or case.
 */
#ifndef __WVHEX_H
#define __WVHEX_H

#include "wvencoder.h"

class WvHexEncoder : public WvEncoder
{
    char alphabase;
public:
    WvHexEncoder(bool use_uppercase = false);
    virtual ~WvHexEncoder() { }

    // on flush, outputs any needed pad characters
    virtual bool encode(WvBuffer &in, WvBuffer &out, bool flush);
};


class WvHexDecoder : public WvEncoder
{
    bool iserror;
    bool issecond;
    int first;
public:
    WvHexDecoder();
    virtual ~WvHexDecoder() { }

    // returns false if an error was detected in the input
    virtual bool isok() const;
    virtual bool encode(WvBuffer &in, WvBuffer &out, bool flush);
};

/*** For compatibility with older code. ***/

/**
 * Write the contents of the binary string of length 'len' pointed to by 'ibuf' 
 * into the output buffer 'obuf' in hexadecimal format.
 * 
 * For example, if len==4, ibuf=="ABCDEF", then obuf will contain "41424344"
 * with a terminating NULL character.
 * 
 * This is useful to turn arbitrary binary into a simple printable format, so
 * that it can (for example) be written to a WvConf configuration file.
 * 
 * obuf must be a buffer with at least (len * 2) + 1 bytes available. (two
 * digits for each byte of ibuf, plus a terminating NULL).
 */
void hexify(char *obuf, const void *ibuf, size_t len);

/**
 * Reverse the operation performed by hexify(). obuf must be a buffer large
 * enough to contain the entire binary output string; you can calculate this
 * size with (strlen(ibuf) / 2). obuf will NOT be automatically NULL-terminated.
 */
void unhexify(void *obuf, const char *ibuf);

#endif // __WVHEX_H
