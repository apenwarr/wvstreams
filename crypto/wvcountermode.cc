/*
 * Worldvisions Tunnel Vision Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * A 'counter mode' cryptography engine abstraction.
 */
#include "wvcountermode.h"


WvCounterModeEncoder::WvCounterModeEncoder(WvEncoder *_keycrypt,
    const void *_counter, size_t _countersize) :
    keycrypt(_keycrypt), counter(NULL)
{
    setcounter(_counter, _countersize);
}


WvCounterModeEncoder::~WvCounterModeEncoder()
{
    delete keycrypt;
    delete[] counter;
}


void WvCounterModeEncoder::setcounter(const void *_counter, size_t _countersize)
{
    delete[] counter;
    counter = new unsigned char[_countersize];
    countersize = _countersize;
    memcpy(counter, _counter, countersize);
}


void WvCounterModeEncoder::getcounter(void *_counter) const
{
    memcpy(_counter, counter, countersize);
}


void WvCounterModeEncoder::incrcounter()
{
    for (size_t i = 0; i < countersize && ! ++counter[i]; ++i);
}


bool WvCounterModeEncoder::_encode(WvBuffer &inbuf, WvBuffer &outbuf,
    bool flush)
{
    bool success = true;
    size_t avail = inbuf.used();
    size_t offset = outbuf.used();
    
    // generate a key stream
    size_t len;
    for (len = avail; len >= countersize; len -= countersize)
    {
        counterbuf.reset(counter, countersize);
        success = keycrypt->encode(counterbuf, outbuf, true);
        if (! success) break;
        incrcounter();
    }
    if (flush && len != 0 && success)
    {
        counterbuf.reset(counter, countersize);
        success = keycrypt->encode(counterbuf, outbuf, true);
        if (success)
        {
            outbuf.unalloc(countersize - len);
            len = 0;
            incrcounter();
        }
        else
            outbuf.unalloc(outbuf.used() - offset - avail);
    }
    avail -= len;
    
    // XOR in the data
    while (avail > 0)
    {
        unsigned char *dataout = outbuf.mutablepeek(offset, & len);
        size_t lenopt = inbuf.usedopt();
        if (len > lenopt)
            len = lenopt;
        const unsigned char *datain = inbuf.get(len);
        
        if (len >= avail)
        {
            len = avail;
            avail = 0;
        }
        else
        {
            avail -= len;
            offset += len;
        }
        while (len-- > 0)
            *(dataout++) ^= *(datain++);
    }
    return success;
}
