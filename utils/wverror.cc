/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * A class for managing error numbers and strings.  See wverror.h.
 */
#include "wverror.h"
#include <assert.h>


WvError::~WvError()
{
    // nothing special
}


const char *WvError::errstr() const
{
    if (errnum == -1)
    {
	assert(!!errstring);
	return errstring;
    }
    else
	return strerror(errnum);
}


void WvError::seterr(int _errnum)
{
    if (!errnum)
	errnum = _errnum;
}


void WvError::seterr(WvStringParm specialerr)
{
    if (!errnum)
    {
	errstring = specialerr;
	seterr(-1);
    }
}


void WvError::seterr(const WvError &err)
{
    if (err.geterr() > 0)
	seterr(err.geterr());
    else if (err.geterr() < 0)
	seterr(err.errstr());
}
