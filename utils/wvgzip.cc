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


bool WvGzip::encode(WvBuffer &in, WvBuffer &out, bool flush)
{
    assert(zstr->avail_in == 0);
    zstr->avail_in = in.used();
    zstr->next_in = in.get(in.used());

    int retval;
    do
    {
	if (zstr->avail_out == 0)
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
        
	size_t tmpused = tmpbuf.used();
	out.put(tmpbuf.get(tmpused), tmpused);
    } while (retval == Z_OK && zstr->avail_out == 0);

    if (retval != Z_OK && retval != Z_BUF_ERROR)
        okay = false;
    return okay;
}
