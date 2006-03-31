/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2004-2006 Net Integration Technologies, Inc.
 * 
 */ 
#include "wvdbuswatch.h"


WvDBusWatch::WvDBusWatch(DBusWatch *_watch, unsigned int _flags) :
    watch(_watch),
    flags(_flags)
{
    int fd = dbus_watch_get_fd(watch);
    rfd = wfd = fd;

//     else if (flags & DBUS_WATCH_WRITABLE)
//         wfd = fd;
}


void WvDBusWatch::execute()
{
    unsigned int dbus_condition = 0;
//     fprintf(stderr, "Execute. %i %i\n", isreadable(), iswritable());

    if (isreadable())
        dbus_condition |= DBUS_WATCH_READABLE;
    if (iswritable())
         dbus_condition |= DBUS_WATCH_WRITABLE;
    // FIXME: Handle errors, HUP

    dbus_watch_handle(watch, dbus_condition);
}
