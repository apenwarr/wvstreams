/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 * 
 * Performs C-style backslash escaping and unescaping of strings.
 */
#include <ctype.h>
#include "wvbackslash.h"

static const char *escapein = "\a\b\f\n\r\t\v";
static const char *escapeout = "abfnrtv";

static inline char tohex(int digit, char alphabase = ('a' - 10))
{
    return (digit < 10 ? '0' : alphabase) + digit;
}

static inline int fromhex(char digit)
{
    if (isdigit(digit))
        return digit - '0';
    if (digit >= 'A' && digit <= 'F')
        return digit - 'A' + 10;
    if (digit >= 'a' && digit <= 'f')
        return digit - 'a' + 10;
    return -1;
}

static inline int fromoctal(char digit)
{
    if (digit >= '0' && digit <= '7')
        return digit - '0';
    return -1;
}


/***** WvBackslashEncoder *****/

WvBackslashEncoder::WvBackslashEncoder(WvStringParm _nasties) :
    nasties(_nasties)
{
}


bool WvBackslashEncoder::_encode(WvBuf &inbuf, WvBuf &outbuf,
    bool flush)
{
    size_t avail = outbuf.free();
    size_t len;
    while ((len = inbuf.optgettable()) != 0)
    {
        const unsigned char *datain = inbuf.get(len);
        for (size_t i = 0; i < len; ++i)
        {
            int c = datain[i];
            
            // handle 1 character escape sequences
            if (avail < 1)
                { outbuf.unget(len - i); return ! flush; }
            const char *foundnasty = NULL;
            const char *foundspecial = NULL;
            if (c != '\0')
            {
                foundnasty = strchr(nasties.cstr(), c);
                if (! foundnasty)
                {
                    foundspecial = strchr(escapein, c);
                    if (! foundspecial && isprint(c))
                    {
                        outbuf.putch(c);
                        avail -= 1;
                        continue;
                    }
                }
            }
            
            // handle 2 character escape sequences
            if (avail < 2)
                { outbuf.unget(len - i); return ! flush; }
            if (foundnasty != NULL)
            {
                outbuf.putch('\\');
                outbuf.putch(c);
                avail -= 2;
                continue;
            }
            if (foundspecial != NULL)
            {
                outbuf.putch('\\');
                outbuf.putch(escapeout[foundspecial - escapein]);
                avail -= 2;
                continue;
            }

            // handle 4 character escape sequences
            if (avail < 4)
                { outbuf.unget(len - i); return ! flush; }
            outbuf.put("\\x", 2);
            outbuf.putch(tohex(c >> 4));
            outbuf.putch(tohex(c & 15));
            avail -= 4;
        }
    }
    return true;
}


bool WvBackslashEncoder::_reset()
{
    return true;
}


/***** WvBackslashDecoder *****/

WvBackslashDecoder::WvBackslashDecoder() : tmpbuf(4)
{
    _reset();
}


bool WvBackslashDecoder::_encode(WvBuf &inbuf, WvBuf &outbuf,
    bool flush)
{
    if (outbuf.free() == 0)
        return inbuf.used() == 0;
    if (! flushtmpbuf(outbuf))
        return false;

    size_t len;
    while ((len = inbuf.optgettable()) != 0)
    {
        const unsigned char *datain = inbuf.get(len);
        for (size_t i = 0; i < len; ++i)
        {
            int c = datain[i];

            switch (state)
            {
                case Initial:
                    if (c == '\\')
                        state = Escape;
                    tmpbuf.putch(c);
                    break;
                
                case Escape:
                    if (c >= '0' && c <= '3')
                    {
                        tmpbuf.unalloc(1);
                        value = c - '0';
                        state = Octal1;
                    }
                    else if (c == 'x')
                    {
                        tmpbuf.putch(c);
                        state = Hex1;
                    }
                    else if (c == '\n')
                    {
                        // line continuation sequence
                        tmpbuf.unalloc(1);
                        tmpbuf.putch('\n');
                        state = Initial;
                    }
                    else
                    {
                        const char *found = strchr(escapeout, c);
                        tmpbuf.unalloc(1);
                        if (found != NULL)
                            c = escapein[found - escapeout];
                        // else we just drop the backslash
                        tmpbuf.putch(c);
                        state = Initial;
                    }
                    break;

                case Hex2:
                case Hex1: {
                    int digit = fromhex(c);
                    if (digit >= 0)
                    {
                        if (state == Hex1)
                        {
                            tmpbuf.unalloc(2);
                            value = digit;
                            state = Hex2;
                        }
                        else
                        {
                            value = (value << 4) | digit;
                            state = Initial;
                        }
                    }
                    else
                    {
                        i -= 1;
                        state = Initial;
                    }
                    break;
                }

                case Octal3:
                case Octal2:
                case Octal1: {
                    int digit = fromoctal(c);
                    if (digit >= 0)
                    {
                        value = (value << 3) | digit;
                        if (state != Octal3)
                            state = State(state + 1);
                        else
                            state = Initial;
                    }
                    else
                    {
                        i -= 1;
                        state = Initial;
                    }
                    break;
                }
            }

            flushtmpbuf(outbuf);
            if (outbuf.free() == 0)
            {
                inbuf.unget(len - i);
                break;
            }
        }
    }
    if (flush)
    {
        if (inbuf.used() != 0)
            return false;
        state = Initial;
        return flushtmpbuf(outbuf);
    }
    return true;

}


bool WvBackslashDecoder::_reset()
{
    state = Initial;
    value = -1;
    tmpbuf.zap();
    return true;
}


bool WvBackslashDecoder::flushtmpbuf(WvBuf &outbuf)
{
    if (state != Initial)
        return true;
        
    if (value != -1)
    {
        tmpbuf.putch(value);
        value = -1;
    }
    
    size_t len = tmpbuf.used();
    if (len == 0)
        return true;
    size_t avail = outbuf.free();
    if (avail > len)
        avail = len;
    outbuf.merge(tmpbuf, avail);
    len -= avail;
    if (len == 0)
    {
        tmpbuf.zap();
        return true;
    }
    return false;
}
