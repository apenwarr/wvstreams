/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2005 Net Integration Technologies, Inc.
 * 
 */ 
#ifndef __IWVDBUSLISTENER_H
#define __IWVDBUSLISTENER_H
#include "wvcallback.h"
#include "wvstring.h"

#include <assert.h>
#include <dbus/dbus.h>

class WvDBusConn;

class IWvDBusListener
{
public:
    IWvDBusListener(WvStringParm _member) { member = _member; }
    virtual ~IWvDBusListener() {}
    virtual void dispatch(DBusMessage *_msg) = 0;

    WvString member;
};

#endif // __IWVDBUSLISTENER_H
