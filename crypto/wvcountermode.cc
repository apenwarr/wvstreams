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
    size_t len;
    while ((len = inbuf.used()) > 0)
    {
        if (len < countersize && ! flush)
            return true;

        // generate a key stream
        counterbuf.zap();
        counterbuf.put(counter, countersize);
        keybuf.zap();
        bool success = keycrypt->encode(counterbuf, keybuf, true);
        if (! success)
            return false;

        // XOR it with the data
        if (len > countersize)
            len = countersize;
        unsigned char *crypt = keybuf.get(len);
        unsigned char *datain = inbuf.get(len);
        unsigned char *dataout = outbuf.alloc(len);
        while (len-- > 0)
            *(dataout++) = *(datain++) ^ *(crypt++);

        // update the counter
        incrcounter();
    }
    return true;
}
