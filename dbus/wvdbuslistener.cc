/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2005-2006 Net Integration Technologies, Inc.
 * 
 */ 
#include "wvdbuslistener.h"


static bool validate_arg_type(WvError &err,
			      DBusMessageIter *iter, int expected_type)
{
    int type = dbus_message_iter_get_arg_type(iter);
    if (type == expected_type)
        return true;

    err.set_both(EINVAL, WvString("Argument type invalid: expected %c, got %c",
		 expected_type, type));
    return false;
}

#define CONVERT_FN(_wvdbus_type, _dbus_type)                            \
    void convert_next(DBusMessageIter *iter, _wvdbus_type &t, WvError &err) \
    {                                                                   \
        if (!validate_arg_type(err, iter, _dbus_type))                  \
            return;                                                     \
                                                                        \
        _wvdbus_type x;                                                 \
        dbus_message_iter_get_basic(iter, &x);                          \
        t = x;                                                          \
        dbus_message_iter_next(iter);                                   \
    }

CONVERT_FN(bool, DBUS_TYPE_BOOLEAN);
CONVERT_FN(char, DBUS_TYPE_BYTE);
CONVERT_FN(int16_t, DBUS_TYPE_INT16);
CONVERT_FN(int32_t, DBUS_TYPE_INT32);
CONVERT_FN(uint16_t, DBUS_TYPE_UINT16);
CONVERT_FN(uint32_t, DBUS_TYPE_UINT32);
CONVERT_FN(double, DBUS_TYPE_DOUBLE);

// strings are a bit of a special case
void convert_next(DBusMessageIter *iter, WvString &s, WvError &err)
{
    if (!validate_arg_type(err, iter, DBUS_TYPE_STRING))
        return;

    char *tmp;
    dbus_message_iter_get_basic(iter, &tmp);
    
    s = tmp;
    dbus_message_iter_next(iter);
}


