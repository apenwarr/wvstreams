/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2005-2006 Net Integration Technologies, Inc.
 * 
 */ 
#include "wvdbusmarshaller.h"


// FIXME: There is no checking at all here! Bad bad bad!


bool convert_next(DBusMessageIter *iter, int &i)
{
    dbus_int32_t x;
    dbus_message_iter_get_basic(iter, &x);
    
    i = x;
    return true;
}


bool convert_next(DBusMessageIter *iter, uint32_t &i)
{
    dbus_int32_t x;
    dbus_message_iter_get_basic(iter, &x);
    
    i = x;
    return true;
}


bool convert_next(DBusMessageIter *iter, WvString &s)
{
    char *tmp;
    dbus_message_iter_get_basic(iter, &tmp);
    
    s = tmp;
    return true;
}
