/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Hex encoder and decoder.
 */
#include "wvhex.h"
#include <ctype.h>

static inline char tohex(int digit, char alphabase)
{
    return (digit < 10 ? '0' : alphabase) + digit;
}

static inline int fromhex(char digit)
{
    if (isdigit(digit))
        return digit - '0';
    if (isupper(digit))
        return digit - 'A' + 10;
    return digit - 'a' + 10;
}

/***** WvHexEncoder *****/

WvHexEncoder::WvHexEncoder(bool use_uppercase) 
{
    alphabase = (use_uppercase ? 'A' : 'a') - 10;
}


bool WvHexEncoder::encode(WvBuffer &in, WvBuffer &out, bool flush)
{
    while (in.used() != 0)
    {
        unsigned char byte = in.getch();
        out.putch(tohex(byte >> 4, alphabase));
        out.putch(tohex(byte & 15, alphabase));
    }
    return true;
}


/***** WvHexDecoder *****/

WvHexDecoder::WvHexDecoder() :
    iserror(false), issecond(false), first(0)
{
}


bool WvHexDecoder::isok() const
{
    return ! iserror;
}


bool WvHexDecoder::encode(WvBuffer &in, WvBuffer &out, bool flush)
{
    if (iserror)
        return false;
        
    while (in.used() != 0)
    {
        char ch = (char) in.getch();
        if (isxdigit(ch))
        {
            int digit = fromhex(ch);
            if ( (issecond = ! issecond) )
                first = digit;
            else
                out.putch(first << 4 | digit);
            continue;
        }
        if (isspace(ch))
            continue;
        // invalid character encountered
        iserror = true;
        return false;
    }
    if (flush && issecond)
        return false; // not enough hex digits supplied
    return true;
}


void hexify(char *obuf, const void *ibuf, size_t len)
{
    size_t outlen = len * 2 + 1;
    WvHexEncoder(false /*use_uppercase*/).
        flush(ibuf, len, obuf, & outlen);
    obuf[outlen] = '\0';
}


void unhexify(void *obuf, const char *ibuf)
{
    size_t inlen = strlen(ibuf);
    size_t outlen = inlen / 2;
    WvHexDecoder().flush(ibuf, inlen, obuf, & outlen);
}
