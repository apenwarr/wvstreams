/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2004-2006 Net Integration Technologies, Inc.
 * 
 */ 
#ifndef __WVDBUSWATCH_H
#define __WVDBUSWATCH_H
#include "wvfdstream.h"
#include "wvlog.h"
#include <dbus/dbus.h>


class WvDBusWatch : public WvFdStream
{
public:
    WvDBusWatch(DBusWatch *_watch, unsigned int _flags);
    virtual void execute();
    // disable reading/writing: we want dbus to do that for us
    virtual size_t uread(void *buf, size_t count) { return 0; }
    virtual size_t uwrite(const void *buf, size_t count) { return 0; }
    virtual bool pre_select(SelectInfo &si);
    DBusWatch *watch;

private:
    WvLog log;
};

#endif // __WVDBUSWATCH_H
