/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Helper(s) to make WvString co-operate better with QString.
 */
#include "wvstring.h"
#include <qstring.h>

#include <stdio.h>

WvFastString::WvFastString(const QString &s)
{
    fprintf(stderr, "ffqs: '%s'\n", s.latin1());
    
#if 1
    link(&nullbuf, NULL);
    *this = WvString(s);
#else
    // just copy the pointer - no need to allocate memory!
    str = (char *)s.latin1(); // I promise not to change anything!
    buf = NULL;
#endif
}


WvFastString::WvFastString(const QCString &s)
{
    fprintf(stderr, "ffqcs: '%s'\n", (const char *)s);
    
#if 1
    link(&nullbuf, NULL);
    *this = WvString(s);
#else
    // just copy the pointer - no need to allocate memory!
    str = (char *)(const char *)s; // I promise not to change anything!
    buf = NULL;
#endif
}


WvFastString::operator QString () const
{
    return cstr();
}


WvString::WvString(const QString &s)
{
    fprintf(stderr, "ssqs: '%s'\n", s.latin1());
    
    link(&nullbuf, s);
    unique();
}


WvString::WvString(const QCString &s)
{
    fprintf(stderr, "ssqcs: '%s'\n", (const char *)s);
    
    link(&nullbuf, s);
    unique();
}
