/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2001 Net Integration Technologies, Inc.
 * 
 * A top-level data encoder class.  See wvencoder.h.
 */
#include "wvencoder.h"

WvEncoder::WvEncoder()
{
    // nothing special
}


WvEncoder::~WvEncoder()
{
    // nothing special
}


bool WvEncoder::isok() const
{
    // most encoders will always be okay
    return true;
}


void WvEncoder::encode(const void *in, size_t insize, bool flush)
{
    size_t len = 0;
    bool go_once = flush;
    
    // repeat until the entire buffer is used
    while (len < insize || go_once)
    {
	go_once = false;
	len += do_encode((const unsigned char *)in, insize, flush);
	//fprintf(stderr, "encoder: %u/%u bytes (buffer now has %d bytes)\n",
	//           len, insize, outbuf.used());
    }
}


