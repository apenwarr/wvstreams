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
    _reset();
}


bool WvHexEncoder::_reset()
{
    return true;
}


bool WvHexEncoder::_encode(WvBuffer &in, WvBuffer &out, bool flush)
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

WvHexDecoder::WvHexDecoder()
{
    _reset();
}


bool WvHexDecoder::_reset()
{
    issecond = false;
    first = 0;
    return true;
}


bool WvHexDecoder::_encode(WvBuffer &in, WvBuffer &out, bool flush)
{
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
        seterror("invalid character '%s' in hex input", ch);
        return false;
    }
    if (flush && issecond)
        return false; // not enough hex digits supplied
    return true;
}


/*** Compatibility ***/

void hexify(char *obuf, const void *ibuf, size_t len)
{
    size_t outlen = len * 2 + 1;
    WvHexEncoder(false /*use_uppercase*/).
        flushmemmem(ibuf, len, obuf, & outlen);
    obuf[outlen] = '\0';
}


void unhexify(void *obuf, const char *ibuf)
{
    size_t inlen = strlen(ibuf);
    size_t outlen = inlen / 2;
    WvHexDecoder().flushmemmem(ibuf, inlen, obuf, & outlen);
}
