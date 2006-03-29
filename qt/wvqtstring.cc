/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Helper(s) to make WvString co-operate better with QString.
 */
#include "wvstring.h"
#include <QtCore/qstring.h>
//#include <Qt/q3cstring.h>
//#include <QtCore/qglobal.h>
#include <stdio.h>

WvFastString::WvFastString(const QString &s)
{
//    fprintf(stderr, "ffqs: '%s'\n", s.latin1());
    
#if 1
    link(&nullbuf, NULL);
    *this = WvString(s);
#else
    // just copy the pointer - no need to allocate memory!
    str = (char *)s.latin1(); // I promise not to change anything!
    buf = NULL;
#endif
}

/*
WvFastString::WvFastString(const Q3CString &s)
{
//    fprintf(stderr, "ffqcs: '%s'\n", (const char *)s);
    
#if 1
    link(&nullbuf, NULL);
    *this = WvString(s);
#else
    // just copy the pointer - no need to allocate memory!
    str = (char *)(const char *)s; // I promise not to change anything!
    buf = NULL;
#endif
}
*/

WvFastString::operator QString () const
{
    return cstr();
}


WvString::WvString(const QString &s)
{
//    fprintf(stderr, "ssqs: '%s'\n", s.latin1());
    
    //link(&nullbuf, qPrintable(s));
    link(&nullbuf, s.toAscii().constData());
    unique();
}

/*
WvString::WvString(const Q3CString &s)
{
// fprintf(stderr, "ssqcs: '%s'\n", (const char *)s);
    
    link(&nullbuf, s.constData());
    unique();
}
*/
