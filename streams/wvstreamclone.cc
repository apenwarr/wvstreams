/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997, 1998 Worldvisions Computer Technology, Inc.
 * 
 * WvStreamClone simply forwards all requests to the "cloned" stream.
 * 
 * See wvstreamclone.h.
 */
#include "wvstreamclone.h"
#include <errno.h>


WvStreamClone::~WvStreamClone()
{
    // do NOT close the cloned stream!
}


void WvStreamClone::close()
{
    if (s())
	s()->close();
}


int WvStreamClone::getfd() const
{
    if (s())
	return s()->getfd();
    return -1;
}


size_t WvStreamClone::uread(void *buf, size_t size)
{
    if (s())
	return s()->read(buf, size);
    else
	return 0;
}


size_t WvStreamClone::uwrite(const void *buf, size_t size)
{
    if (s())
	return s()->write(buf, size);
    else
	return 0;
}


bool WvStreamClone::isok() const
{
    if (errnum)
	return false;
    if (s())
	return s()->isok();
    return false;
}


int WvStreamClone::geterr() const
{
    if (errnum)
	return errnum;
    if (s())
	return s()->geterr();
    return EIO;
}


const char *WvStreamClone::errstr() const
{
    if (errnum)
	return WvStream::errstr();
    if (s())
	return s()->errstr();
    return "No child stream!";
}


bool WvStreamClone::select_setup(SelectInfo &si)
{
    if (si.readable && !select_ignores_buffer && inbuf.used() 
	   && inbuf.used() >= queue_min)
	return true;   // sure_thing if anything in WvStream buffer

    if (s() && s()->isok())
	return s()->select_setup(si);
    return false;
}


bool WvStreamClone::test_set(SelectInfo &si)
{
    if (s() && s()->isok())
	return s()->test_set(si);
    return false;
}


const WvAddr *WvStreamClone::src() const
{
    if (s())
	return s()->src();
    return NULL;
}


void WvStreamClone::execute()
{
    if (s()) s()->callback();
}
