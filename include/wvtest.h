/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */

/** \file
 * The beginnings of an automated testing framework.  Makes WvLog data
 * come out in a particular format that should be useful for greps and
 * automated comparisons.
 */ 
#ifndef __WVTEST_H
#define __WVTEST_H

#include "wvlogrcv.h"

class WvTestRcv : public WvLogConsole
{
public:
    WvTestRcv();
    virtual ~WvTestRcv();
    
protected:
    int testnum;
    bool testing;
    
    virtual void _begin_line();
    virtual void _end_line();
};


class WvTest : public WvLog
{
    WvTestRcv testrcv;
    
public:
    WvTest();
    virtual ~WvTest();
};



#endif // __WVTEST_H
