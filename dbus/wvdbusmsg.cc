/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2004-2006 Net Integration Technologies, Inc.
 * 
 */ 
#include "wvdbusmsg.h"


WvDBusMsg::WvDBusMsg(WvStringParm busname, WvStringParm objectname, 
                     WvStringParm interface, WvStringParm method)
{
    msg = dbus_message_new_method_call(busname, objectname, interface, method);
}


void WvDBusMsg::append(WvStringParm s1)
{
    assert(msg);
    const char *tmp;
    if (!s1.isnull())
    {
	tmp = s1.cstr();
	dbus_message_append_args(msg, DBUS_TYPE_STRING, &tmp,
                                 DBUS_TYPE_INVALID);
    }
}


void WvDBusMsg::append(bool b)
{
    assert(msg);
    dbus_message_append_args(msg, DBUS_TYPE_BOOLEAN, &b, DBUS_TYPE_INVALID);
}


void WvDBusMsg::append(char c)
{
    assert(msg);
    dbus_message_append_args(msg, DBUS_TYPE_BYTE, &c, DBUS_TYPE_INVALID);
}


void WvDBusMsg::append(int16_t i)
{
    assert(msg);
    dbus_message_append_args(msg, DBUS_TYPE_INT16, &i, DBUS_TYPE_INVALID);
}


void WvDBusMsg::append(uint16_t i)
{
    assert(msg);
    dbus_message_append_args(msg, DBUS_TYPE_UINT16, &i, DBUS_TYPE_INVALID);
}


void WvDBusMsg::append(int32_t i)
{
    assert(msg);
    dbus_message_append_args(msg, DBUS_TYPE_INT32, &i, DBUS_TYPE_INVALID);
}


void WvDBusMsg::append(uint32_t i)
{
    assert(msg);
    dbus_message_append_args(msg, DBUS_TYPE_UINT32, &i, DBUS_TYPE_INVALID);
}


void WvDBusMsg::append(double d)
{
    assert(msg);
    dbus_message_append_args(msg, DBUS_TYPE_DOUBLE, &d, DBUS_TYPE_INVALID);
}


WvDBusReplyMsg::WvDBusReplyMsg(DBusMessage *_msg) 
{
    assert(msg);
    msg = dbus_message_new_method_return(_msg);
}


WvDBusSignal::WvDBusSignal(WvStringParm objectname, WvStringParm interface,
                           WvStringParm name)
{
    msg = dbus_message_new_signal(objectname, interface, name);
}
