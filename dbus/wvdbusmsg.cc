/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2004-2006 Net Integration Technologies, Inc.
 * 
 * Pathfinder Software:
 *   Copyright (C) 2007, Carillon Information Security Inc.
 *
 * This library is licensed under the LGPL, please read LICENSE for details.
 *
 */ 
#include "wvdbusmsg.h"
#include <dbus/dbus.h>


WvDBusMsg::Iter::Iter(const WvDBusMsg &_msg) : msg(_msg)
{
    it = NULL;
    rewind();
}

WvDBusMsg::Iter::~Iter()
{
    if (it)
	delete it;
}


/**
 * Rewinds the iterator to make it point to an imaginary element
 * preceeding the first element of the list.
 */
void WvDBusMsg::Iter::rewind()
{
    rewound = true;
    if (it)
	delete it;
    it = new DBusMessageIter;
}


int WvDBusMsg::Iter::type() const
{
    return dbus_message_iter_get_arg_type(it);
}


/**
 * Moves the iterator along the list to point to the next element.
 * 
 * If the iterator had just been rewound, it now points to the
 * first element of the list.
 */
bool WvDBusMsg::Iter::next()
{
    if (rewound)
	dbus_message_iter_init(msg, it);
    else
	dbus_message_iter_next(it);
    rewound = false;
    return type() != DBUS_TYPE_INVALID;
}


/**
 * Returns: true if the current link is valid
 */
bool WvDBusMsg::Iter::cur() const
{
    return !rewound && type() != DBUS_TYPE_INVALID;
}


/**
 * Get the current element as a string (possible for all types).
 */
WvString WvDBusMsg::Iter::get_str() const
{
    char *s;
    double d;
    
    switch (type())
    {
    case DBUS_TYPE_BYTE:
    case DBUS_TYPE_BOOLEAN: 
    case DBUS_TYPE_INT16: 
    case DBUS_TYPE_INT32: 
    case DBUS_TYPE_INT64: 
	return get_int();
    case DBUS_TYPE_UINT16: 
    case DBUS_TYPE_UINT32: 
    case DBUS_TYPE_UINT64: 
	return get_uint();
    case DBUS_TYPE_DOUBLE: 
	dbus_message_iter_get_basic(it, &d);
	return (int)d;
    case DBUS_TYPE_STRING: 
	dbus_message_iter_get_basic(it, &s);
	return s;
    default:
	return WvString("UNKNOWN_TYPE(%c)", type());
    }
}


/**
 * Get the current element as an int64_t
 * (possible for all integer types)
 */
int64_t WvDBusMsg::Iter::get_int() const
{
    int8_t b;
    int16_t s;
    int32_t i;
    int64_t l;
    char *str;
    
    switch (type())
    {
    case DBUS_TYPE_BYTE: 
    case DBUS_TYPE_BOOLEAN: 
	dbus_message_iter_get_basic(it, &b);
	return b;
    case DBUS_TYPE_INT16: 
    case DBUS_TYPE_UINT16: 
	dbus_message_iter_get_basic(it, &s);
	return s;
    case DBUS_TYPE_INT32: 
    case DBUS_TYPE_UINT32:
	dbus_message_iter_get_basic(it, &i);
	return i;
	
    case DBUS_TYPE_INT64: 
    case DBUS_TYPE_UINT64: 
	dbus_message_iter_get_basic(it, &l);
	return l;
	
    case DBUS_TYPE_STRING: 
	dbus_message_iter_get_basic(it, &str);
	return WvString(str).num();
    default:
	return 0;
    }
}


/**
 * Get the current element as a uint64_t
 * (possible for all integer types)
 */
uint64_t WvDBusMsg::Iter::get_uint() const
{
    uint8_t b;
    uint16_t s;
    uint32_t i;
    uint64_t l;
    char *str;
    
    switch (type())
    {
    case DBUS_TYPE_BYTE: 
    case DBUS_TYPE_BOOLEAN: 
	dbus_message_iter_get_basic(it, &b);
	return b;
    case DBUS_TYPE_INT16: 
    case DBUS_TYPE_UINT16: 
	dbus_message_iter_get_basic(it, &s);
	return s;
    case DBUS_TYPE_INT32: 
    case DBUS_TYPE_UINT32:
	dbus_message_iter_get_basic(it, &i);
	return i;
	
    case DBUS_TYPE_INT64: 
    case DBUS_TYPE_UINT64: 
	dbus_message_iter_get_basic(it, &l);
	return l;
	
    case DBUS_TYPE_STRING: 
	dbus_message_iter_get_basic(it, &str);
	return WvString(str).num();
    default:
	return 0;
    }
}


/**
 * Returns a pointer to the WvString at the iterator's current
 * location.  Needed so that WvIterStuff() will work.
 */
WvString *WvDBusMsg::Iter::ptr() const
{
    s = get_str();
    return &s;
}





WvDBusMsg::WvDBusMsg(WvStringParm busname, WvStringParm objectname, 
                     WvStringParm interface, WvStringParm method)
{
    msg = dbus_message_new_method_call(busname, objectname, interface, method);
    iter = new DBusMessageIter;
    dbus_message_iter_init_append(msg, iter);
}


WvDBusMsg::WvDBusMsg(WvDBusMsg &_msg)
{
    msg = _msg.msg;
    dbus_message_ref(msg);
    iter = new DBusMessageIter;
    dbus_message_iter_init_append(msg, iter);
}


WvDBusMsg::WvDBusMsg(DBusMessage *_msg)
{
    msg = _msg;
    dbus_message_ref(msg);
    iter = new DBusMessageIter;
    dbus_message_iter_init_append(msg, iter);
}


WvDBusMsg::~WvDBusMsg()
{
    delete iter;
    dbus_message_unref(msg);
}


WvDBusMsg::operator DBusMessage* () const
{
    return msg;
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
    Iter i(*this);
    for (i.rewind(); i.next(); )
	list.append(i.get_str());
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
    dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &s);
}


void WvDBusMsg::append(bool b)
{
    assert(msg);
    dbus_message_iter_append_basic(iter, DBUS_TYPE_BOOLEAN, &b);
}


void WvDBusMsg::append(char c)
{
    assert(msg);
    dbus_message_iter_append_basic(iter, DBUS_TYPE_BYTE, &c);
}


void WvDBusMsg::append(int16_t i)
{
    assert(msg);
    dbus_message_iter_append_basic(iter, DBUS_TYPE_INT16, &i);
}


void WvDBusMsg::append(uint16_t i)
{
    assert(msg);
    dbus_message_iter_append_basic(iter, DBUS_TYPE_UINT16, &i);
}


void WvDBusMsg::append(int32_t i)
{
    assert(msg);
    dbus_message_iter_append_basic(iter, DBUS_TYPE_INT32, &i);
}


void WvDBusMsg::append(uint32_t i)
{
    assert(msg);
    dbus_message_iter_append_basic(iter, DBUS_TYPE_UINT32, &i);
}


void WvDBusMsg::append(double d)
{
    assert(msg);
    dbus_message_iter_append_basic(iter, DBUS_TYPE_DOUBLE, &d);
}


WvDBusReplyMsg::WvDBusReplyMsg(DBusMessage *_msg) 
    : WvDBusMsg(dbus_message_new_method_return(_msg))
{
}


WvDBusSignal::WvDBusSignal(WvStringParm objectname, WvStringParm interface,
                           WvStringParm name)
    : WvDBusMsg(dbus_message_new_signal(objectname, interface, name))
{
}
