/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2005 Net Integration Technologies, Inc.
 * 
 */ 
#ifndef __IWVDBUSLISTENER_H
#define __IWVDBUSLISTENER_H

#include "wvdbusmsg.h"
#include "wvstring.h"

class IWvDBusListener
{
public:
    IWvDBusListener(WvStringParm _member) 
        { member = _member; }
    virtual ~IWvDBusListener() {}
    virtual void dispatch(const WvDBusMsg &_msg) = 0;
    void operator() (const WvDBusMsg &_msg) { dispatch(_msg); }

    /// The method/member name of us inside our DBus object
    WvString member;
};

#endif // __IWVDBUSLISTENER_H
