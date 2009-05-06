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
#include "wvdbusconn.h"
#include "wvstrutils.h"
#undef interface // windows
#include <dbus/dbus.h>


class WvDBusReplyMsg : public WvDBusMsg
{
public:
    /**
     * Constructs a new reply message (a message intended to be a reply to
     * an existing D-Bus message).
     * 
     * Don't call this directly.  Use WvDBusMsg::reply() instead.
     */
    WvDBusReplyMsg(DBusMessage *_msg);

    virtual ~WvDBusReplyMsg() {}
};



WvDBusMsg::Iter::Iter(const WvDBusMsg &_msg)
    : first(new DBusMessageIter), it(new DBusMessageIter)
{
    dbus_message_iter_init(_msg, first);
    rewind();
}


WvDBusMsg::Iter::Iter(const WvDBusMsg::Iter &_it)
    : first(new DBusMessageIter), it(new DBusMessageIter)
{
    *first = *_it.first;
    rewind();
}


WvDBusMsg::Iter::Iter(const DBusMessageIter &_first)
    : first(new DBusMessageIter), it(new DBusMessageIter)
{
    *first = _first;
    rewind();
}


WvDBusMsg::Iter::~Iter()
{
    delete first;
    delete it;
}


void WvDBusMsg::Iter::rewind()
{
    rewound = true;
}


bool WvDBusMsg::Iter::next()
{
    if (rewound)
	*it = *first;
    else if (type() != DBUS_TYPE_INVALID)
	dbus_message_iter_next(it);
    rewound = false;
    return type() != DBUS_TYPE_INVALID;
}


int WvDBusMsg::Iter::type() const
{
    return dbus_message_iter_get_arg_type(it);
}


WvDBusMsg::Iter WvDBusMsg::Iter::open() const
{
    DBusMessageIter sub;
    dbus_message_iter_recurse(it, &sub);
    return Iter(sub); 
}


bool WvDBusMsg::Iter::cur() const
{
    return !rewound && type() != DBUS_TYPE_INVALID;
}


void WvDBusMsg::Iter::get_all(WvStringList &list)
{
    int items = 0;
    for (rewind(); next() && items < 20; items++)
	list.append(get_str());
    if (items == 20)
	list.append("...");
}


WvString WvDBusMsg::Iter::get_all()
{
    WvStringList list;
    get_all(list);
    return list.join(",");
}


WvString WvDBusMsg::Iter::get_str() const
{
    char *s;
    double d;
    
    switch (type())
    {
    case DBUS_TYPE_BYTE:
	// Don't do this: things like vxodbc expect to be able to atoi()
	// the resulting string!
	//return WvString("y%s", get_int());
    case DBUS_TYPE_BOOLEAN: 
	//return WvString("b%s", get_int());
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
	return d;
    case DBUS_TYPE_STRING: 
	dbus_message_iter_get_basic(it, &s);
	return s;
    case DBUS_TYPE_VARIANT:
	return WvString("{%s}", open().getnext().get_str());
    case DBUS_TYPE_STRUCT:
    case DBUS_TYPE_ARRAY:
	return WvString("[%s]", open().get_all());
    case DBUS_TYPE_INVALID:
	return WvString();
    default:
	return WvString("UNKNOWN_TYPE(%c)", type());
    }
}


int64_t WvDBusMsg::Iter::get_int() const
{
    dbus_bool_t b;
    unsigned char c;
    dbus_int16_t s;
    dbus_int32_t i;
    dbus_int64_t l;
    char *str;
    
    switch (type())
    {
    case DBUS_TYPE_BYTE: 
	dbus_message_iter_get_basic(it, &c);
	return c;
	
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
	
    case DBUS_TYPE_VARIANT:
	return open().getnext().get_int();
	
    default:
	return 0;
    }
}


uint64_t WvDBusMsg::Iter::get_uint() const
{
    dbus_bool_t b;
    unsigned char c;
    dbus_uint16_t s;
    dbus_uint32_t i;
    dbus_uint64_t l;
    char *str;
    
    switch (type())
    {
    case DBUS_TYPE_BYTE: 
	dbus_message_iter_get_basic(it, &c);
	return c;
	
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
	
    case DBUS_TYPE_VARIANT:
	return open().getnext().get_uint();
	
    default:
	return 0;
    }
}


double WvDBusMsg::Iter::get_double() const
{
    dbus_bool_t b;
    unsigned char c;
    dbus_uint16_t s;
    dbus_uint32_t i;
    dbus_uint64_t l;
    char *str;
    double d;
    
    switch (type())
    {
    case DBUS_TYPE_DOUBLE:
	dbus_message_iter_get_basic(it, &d);
        return d;

    case DBUS_TYPE_BYTE: 
	dbus_message_iter_get_basic(it, &c);
	return c;
	
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
	return atof(str);
	
    case DBUS_TYPE_VARIANT:
	return open().getnext().get_double();
	
    default:
	return 0;
    }
}


WvString *WvDBusMsg::Iter::ptr() const
{
    s = get_str();
    return &s;
}




static DBusMessageIter *new_append_iter(WvDBusMsg &msg)
{
    DBusMessageIter *it = new DBusMessageIter;
    dbus_message_iter_init_append(msg, it);
    return it;
}


WvDBusMsg::WvDBusMsg(WvStringParm busname, WvStringParm objectname, 
                     WvStringParm interface, WvStringParm method)
{
    msg = dbus_message_new_method_call(busname, objectname, interface, method);
    itlist.prepend(new_append_iter(*this), true);
}


WvDBusMsg::WvDBusMsg(WvDBusMsg &_msg)
{
    msg = _msg.msg;
    dbus_message_ref(msg);
    itlist.prepend(new_append_iter(*this), true);
}


WvDBusMsg::WvDBusMsg(DBusMessage *_msg)
{
    msg = _msg;
    dbus_message_ref(msg);
    itlist.prepend(new_append_iter(*this), true);
}


WvDBusMsg::~WvDBusMsg()
{
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


WvString WvDBusMsg::get_error() const
{
    if (iserror())
	return dbus_message_get_error_name(msg);

    return WvString::null;
}

bool WvDBusMsg::is_reply() const
{
    // This used to have a hack to deal with replies to message #0.
    // But it turns out the first message is #1, so that's not an actual
    // problem.
    return get_replyserial() != 0;
}


uint32_t WvDBusMsg::get_serial() const
{
    return dbus_message_get_serial(msg);
}


uint32_t WvDBusMsg::get_replyserial() const
{
    return dbus_message_get_reply_serial(msg);
}


void WvDBusMsg::get_arglist(WvStringList &list) const
{
    Iter(*this).get_all(list);
}


WvString WvDBusMsg::get_argstr() const
{
    return Iter(*this).get_all();
}


WvDBusMsg::operator WvString() const
{
    WvString dest(get_dest());
    if (!dest)
	dest = "";
    else
	dest = WvString("%s:", dest);
    if (is_reply())
    {
	if (iserror())
	    return WvString("ERR#%s->%s#%s(%s)",
			    get_serial(), dest, get_replyserial(),
			    get_argstr());
	else
	    return WvString("REPLY#%s->%s#%s(%s)",
			    get_serial(), dest, get_replyserial(),
			    get_argstr());
    }
    else
    {
	WvString s("%s%s/%s.%s(%s)#%s",
		   dest,
		   get_path(), get_interface(), get_member(),
		   get_argstr(), get_serial());
	s = strreplace(s, "org.freedesktop.DBus", "o.f.D");
	s = strreplace(s, "org/freedesktop/DBus", "o/f/D");
	return s;
    }
}


WvDBusMsg &WvDBusMsg::append(const char *s)
{
    assert(msg);
    assert(s);
    dbus_message_iter_append_basic(itlist.first(), DBUS_TYPE_STRING, &s);
    return *this;
}


WvDBusMsg &WvDBusMsg::append(bool b)
{
    assert(msg);
    dbus_bool_t bb = b;
    dbus_message_iter_append_basic(itlist.first(), DBUS_TYPE_BOOLEAN, &bb);
    return *this;
}


WvDBusMsg &WvDBusMsg::append(signed char c)
{
    assert(msg);
    dbus_unichar_t cc = c;
    dbus_message_iter_append_basic(itlist.first(), DBUS_TYPE_BYTE, &cc);
    return *this;
}


WvDBusMsg &WvDBusMsg::append(unsigned char c)
{
    assert(msg);
    dbus_unichar_t cc = c;
    dbus_message_iter_append_basic(itlist.first(), DBUS_TYPE_BYTE, &cc);
    return *this;
}


WvDBusMsg &WvDBusMsg::append(int16_t i)
{
    assert(msg);
    dbus_message_iter_append_basic(itlist.first(), DBUS_TYPE_INT16, &i);
    return *this;
}


WvDBusMsg &WvDBusMsg::append(uint16_t i)
{
    assert(msg);
    dbus_message_iter_append_basic(itlist.first(), DBUS_TYPE_UINT16, &i);
    return *this;
}


WvDBusMsg &WvDBusMsg::append(int32_t i)
{
    assert(msg);
    dbus_message_iter_append_basic(itlist.first(), DBUS_TYPE_INT32, &i);
    return *this;
}


WvDBusMsg &WvDBusMsg::append(uint32_t i)
{
    assert(msg);
    dbus_message_iter_append_basic(itlist.first(), DBUS_TYPE_UINT32, &i);
    return *this;
}


WvDBusMsg &WvDBusMsg::append(int64_t i)
{
    assert(msg);
    dbus_message_iter_append_basic(itlist.first(), DBUS_TYPE_INT64, &i);
    return *this;
}


WvDBusMsg &WvDBusMsg::append(uint64_t i)
{
    assert(msg);
    dbus_message_iter_append_basic(itlist.first(), DBUS_TYPE_UINT64, &i);
    return *this;
}


WvDBusMsg &WvDBusMsg::append(double d)
{
    assert(msg);
    dbus_message_iter_append_basic(itlist.first(), DBUS_TYPE_DOUBLE, &d);
    return *this;
}


WvDBusMsg &WvDBusMsg::variant_start(WvStringParm element_type)
{
    DBusMessageIter *parent = itlist.first();
    DBusMessageIter *sub = new DBusMessageIter;
    dbus_message_iter_open_container(parent,
				     DBUS_TYPE_VARIANT, element_type, sub);
    itlist.prepend(sub, true);
    return *this;
}


WvDBusMsg &WvDBusMsg::variant_end()
{
    assert(itlist.count() >= 2);
    
    WvList<DBusMessageIter>::Iter i(itlist);
    i.rewind(); i.next();
    DBusMessageIter *sub = i.ptr();
    i.next();
    DBusMessageIter *parent = i.ptr();
    
    dbus_message_iter_close_container(parent, sub);
    itlist.unlink_first();
    return *this;
}


WvDBusMsg &WvDBusMsg::struct_start(WvStringParm element_type)
{
    DBusMessageIter *parent = itlist.first();
    DBusMessageIter *sub = new DBusMessageIter;
    dbus_message_iter_open_container(parent,
				     DBUS_TYPE_STRUCT, 0, sub);
    itlist.prepend(sub, true);
    return *this;
}


WvDBusMsg &WvDBusMsg::struct_end()
{
    return array_end(); // same thing
}


WvDBusMsg &WvDBusMsg::array_start(WvStringParm element_type)
{
    DBusMessageIter *parent = itlist.first();
    DBusMessageIter *sub = new DBusMessageIter;
    dbus_message_iter_open_container(parent,
				     DBUS_TYPE_ARRAY, element_type, sub);
    itlist.prepend(sub, true);
    return *this;
}


WvDBusMsg &WvDBusMsg::array_end()
{
    return variant_end(); // same thing
}


WvDBusMsg &WvDBusMsg::varray_start(WvStringParm element_type)
{
    variant_start(WvString("a%s", element_type));
    return array_start(element_type);
}


WvDBusMsg &WvDBusMsg::varray_end()
{
    assert(itlist.count() >= 3);
    array_end();
    return variant_end();
}


WvDBusMsg WvDBusMsg::reply()
{
    return WvDBusReplyMsg(*this);
}


bool WvDBusMsg::iserror() const
{
    return dbus_message_get_type(msg) == DBUS_MESSAGE_TYPE_ERROR;
}


void WvDBusMsg::send(WvDBusConn &conn)
{
    conn.send(*this);
}


WvDBusReplyMsg::WvDBusReplyMsg(DBusMessage *_msg) 
    : WvDBusMsg(dbus_message_new_method_return(_msg))
{
    dbus_message_unref(msg);
}


WvDBusSignal::WvDBusSignal(WvStringParm objectname, WvStringParm interface,
                           WvStringParm name)
    : WvDBusMsg(dbus_message_new_signal(objectname, interface, name))
{
    dbus_message_unref(msg);
}


DBusMessage *WvDBusError::setup1(WvDBusMsg &in_reply_to,
			 WvStringParm errname, WvStringParm message)
{
    return dbus_message_new_error(in_reply_to, errname, message);
}

void WvDBusError::setup2()
{
    dbus_message_unref(msg);
}
