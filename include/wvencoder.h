/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * A top-level data encoder class.  You put data in, and when it comes out
 * it's encoded.  It might be a different size or not, and future data might
 * or might not depend on previous data, depending on the encoder type.
 * 
 * Decoders are also derived from WvEncoder, since they're
 * really just encoders too.
 */
#ifndef __WVENCODER_H
#define __WVENCODER_H

#include "wvbuffer.h"

class WvEncoder
{
public:
    WvEncoder();
    virtual ~WvEncoder();
    
    virtual bool isok() const;
    void encode(const void *in, size_t insize, bool flush);
    void flush()
    	{ encode(NULL, 0, true); }
    
    WvBuffer outbuf;
    
protected:
    // note: this function may be called with in==NULL, but only if insize==0
    // and flush==true.
    virtual size_t do_encode(const unsigned char *in, size_t insize,
			     bool flush) = 0;
};

#endif // __WVENCODER_H
