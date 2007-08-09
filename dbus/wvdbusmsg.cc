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


WvString WvDBusMsg::get_sender() const
{
    return dbus_message_get_sender(msg);
}


WvString WvDBusMsg::get_dest() const
{
    return dbus_message_get_destination(msg);
}


WvString WvDBusMsg::get_path() const
{
    return dbus_message_get_path(msg);
}


WvString WvDBusMsg::get_interface() const
{
    return dbus_message_get_interface(msg);
}


WvString WvDBusMsg::get_member() const
{
    return dbus_message_get_member(msg);
}


void WvDBusMsg::get_arglist(WvStringList &list) const
{
    DBusMessageIter i;
    int type;
    
    dbus_message_iter_init(msg, &i);
    while ((type = dbus_message_iter_get_arg_type(&i)) != DBUS_TYPE_INVALID)
    {
	WvString s;
	switch (type)
	{
	case DBUS_TYPE_BYTE: 
	    { char x; dbus_message_iter_get_basic(&i, &x); s = x; }
	    break;
	case DBUS_TYPE_BOOLEAN: 
	    { int x; dbus_message_iter_get_basic(&i, &x); s = x; }
	    break;
	case DBUS_TYPE_INT16: 
	    { int16_t x; dbus_message_iter_get_basic(&i, &x); s = x; }
	    break;
	case DBUS_TYPE_UINT16: 
	    { uint16_t x; dbus_message_iter_get_basic(&i, &x); s = x; }
	    break;
	case DBUS_TYPE_INT32: 
	    { int32_t x; dbus_message_iter_get_basic(&i, &x); s = x; }
	    break;
	case DBUS_TYPE_UINT32: 
	    { uint32_t x; dbus_message_iter_get_basic(&i, &x); s = x; }
	    break;
	case DBUS_TYPE_INT64: 
	    { int64_t x; dbus_message_iter_get_basic(&i, &x); s = x; }
	    break;
	case DBUS_TYPE_UINT64: 
	    { uint64_t x; dbus_message_iter_get_basic(&i, &x); s = x; }
	    break;
	case DBUS_TYPE_DOUBLE: 
	    { double x; dbus_message_iter_get_basic(&i, &x); s = (int)x; }
	    break;
	case DBUS_TYPE_STRING: 
	    { char *x; dbus_message_iter_get_basic(&i, &x); s = x; }
	    break;
	default:
	    s = WvString("UNKNOWN_TYPE(%c)", type);
	    break;
	}
	list.append(s);
	dbus_message_iter_next(&i);
    }
}


WvString WvDBusMsg::get_argstr() const
{
    WvStringList l;
    get_arglist(l);
    return l.join(",");
}


WvDBusMsg::operator WvString() const
{
    WvString src("");
    if (!!get_sender())
	src = WvString("%s->", get_sender());
    return WvString("%s%s:%s:%s:%s(%s)",
		    src, get_dest(),
		    get_path(), get_interface(), get_member(), get_argstr());
}


void WvDBusMsg::append(const char *s)
{
    assert(msg);
    assert(s);
    dbus_message_append_args(msg, DBUS_TYPE_STRING, &s, DBUS_TYPE_INVALID);
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
    assert(_msg);
    msg = dbus_message_new_method_return(_msg);
}


WvDBusSignal::WvDBusSignal(WvStringParm objectname, WvStringParm interface,
                           WvStringParm name)
{
    msg = dbus_message_new_signal(objectname, interface, name);
}
