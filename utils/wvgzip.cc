/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * gzip encoder/decoders based on zlib.
 */
#include "wvgzip.h"
#include <zlib.h>

#include <assert.h>

#define ZBUFSIZE 10240

WvGzip::WvGzip(GzipMode _mode) : tmpbuf(ZBUFSIZE)
{
    int retval;
    
    okay = true;
    mode = _mode;
    zstr = new z_stream;
    
    memset(zstr, 0, sizeof(*zstr));
    zstr->zalloc = Z_NULL;
    zstr->zfree = Z_NULL;
    zstr->opaque = NULL;
    
    if (mode == Compress)
	retval = deflateInit(zstr, Z_DEFAULT_COMPRESSION);
    else
	retval = inflateInit(zstr);
    
    if (retval != Z_OK)
    {
	okay = false;
	return;
    }
    
    zstr->next_in = zstr->next_out = NULL;
    zstr->avail_in = zstr->avail_out = 0;
}


WvGzip::~WvGzip()
{
    deflateEnd(zstr);
    delete zstr;
}


bool WvGzip::isok() const
{
    return okay;
}


size_t WvGzip::do_encode(const unsigned char *in, size_t insize, bool flush)
{
    assert(!zstr->avail_in && (insize || flush));
    
    int retval;
    size_t taken = 0, tmpused;
    
    if (in && !zstr->avail_in)
    {
	zstr->avail_in = insize;
	zstr->next_in = (unsigned char *)in;
    }

    do
    {
	if (!zstr->avail_out)
	{
	    tmpbuf.zap();
	    assert(tmpbuf.free() == ZBUFSIZE);
	    zstr->avail_out = tmpbuf.free();
	    zstr->next_out = tmpbuf.alloc(tmpbuf.free());
	}
	
	tmpbuf.alloc(tmpbuf.free());
	if (mode == Compress)
	    retval = deflate(zstr, flush ? Z_SYNC_FLUSH : Z_NO_FLUSH);
	else
	    retval = inflate(zstr, flush ? Z_SYNC_FLUSH : Z_NO_FLUSH);
	tmpbuf.unalloc(zstr->avail_out);
	
	//fprintf(stderr, "avail_in: %d, avail_out: %d\n", zstr->avail_in,
	//            zstr->avail_out);
	
	taken = insize - zstr->avail_in;
	
	tmpused = tmpbuf.used();
	outbuf.put(tmpbuf.get(tmpused), tmpused);
	
	fprintf(stderr, "obu: %d\n", outbuf.used());
    } while (retval == Z_OK && !zstr->avail_out);
	
    if (retval != Z_OK && retval != Z_BUF_ERROR)
    {
	fprintf(stderr, "gzip: retval was %d!\n", retval);
	okay = false;
    }
    return taken;
}
