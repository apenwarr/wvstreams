/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */

/** \file
 * Hex encoder and hex decoder.
 */
#ifndef __WVHEX_H
#define __WVHEX_H

#include "wvencoder.h"

/**
 * A hex encoder.
 * <p>
 * The input data is transformed into a sequence of hexadecimal
 * characters.
 * </p><p>
 * Supports reset().
 * </p>
 */
class WvHexEncoder : public WvEncoder
{
    char alphabase;

public:
    /**
     * Creates a hex encoder.
     *
     * @param use_uppercase if true, outputs hex codes A through Z
     *        in upper case, otherwise output them in lower case
     *        (the default)
     */
    WvHexEncoder(bool use_uppercase = false);
    virtual ~WvHexEncoder() { }

protected:
    virtual bool _encode(WvBuffer &in, WvBuffer &out, bool flush);
    virtual bool _reset(); // supported
};


/**
 * A hex decoder.
 * <p>
 * The input hex characters are paired and decoded into the
 * corresponding byte stream.  Whitespace is skipped as is the
 * case of the hex codes A through Z.  Other characters cause the
 * encoder to flag an error.
 * </p><p>
 * Supports reset().
 * </p>
 */
class WvHexDecoder : public WvEncoder
{
    bool issecond;
    int first;
    
public:
    /**
     * Creates a hex decoder.
     */
    WvHexDecoder();
    virtual ~WvHexDecoder() { }

protected:
    virtual bool _encode(WvBuffer &in, WvBuffer &out, bool flush);
    virtual bool _reset(); // supported
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