/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * A class for managing error numbers and strings.  See wverror.h.
 */
#include "wverror.h"
#include <assert.h>

#ifdef _WIN32
#include "windows.h"
#endif

WvError::~WvError()
{
    // nothing special
}


WvString WvError::errstr() const
{
    if (errnum == -1)
    {
	assert(!!errstring);
	return errstring;
    }
    else
    {
#ifndef _WIN32
	return strerror(errnum);
#else
	char msg[4096];
	const HMODULE module = GetModuleHandle("winsock.dll");
	DWORD result = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, module, errnum, 0, msg, sizeof(msg), 0);
	if (result)
	    return msg;
	else
	{
	    DWORD e = GetLastError();
	    return "Unknown error";
	}
#endif
    }
}


void WvError::seterr(int _errnum)
{
    assert(_errnum != -1 || !!errstring &&
            "attempt to set errnum to -1 without also setting errstring");
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
