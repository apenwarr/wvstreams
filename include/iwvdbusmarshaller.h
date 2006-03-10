/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2005 Net Integration Technologies, Inc.
 * 
 */ 
#ifndef __IWVDBUSMARSHALLER_H
#define __IWVDBUSMARSHALLER_H
#include "wvcallback.h"
#include "wvstring.h"

#include <assert.h>
#include <dbus/dbus.h>

class WvDBusConn;

class IWvDBusMarshaller
{
public:
    IWvDBusMarshaller(WvStringParm _path) { path = _path; }
    virtual ~IWvDBusMarshaller() {}
    virtual void dispatch(DBusMessage *_msg) = 0;

    WvString path;
};

#endif // __IWVDBUSMARSHALLER_H
