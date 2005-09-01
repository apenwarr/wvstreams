/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 2003-2005 Net Integration Technologies, Inc.
 *
 * A wrapper around the D-BUS message-passing interface.
 */

#include "wvdbus.h"
#include "wvstream.h"

dbus_bool_t WvDBus::add_watch(DBusWatch *watch, void *data)
{
  
    WvDBus *stream = static_cast<WvDBus *>(data);

    if (stream == NULL)
        return FALSE;

    int fd = dbus_watch_get_fd(watch);
    stream->setfd(fd);

    fprintf(stderr, "Watch added successfully (fd: %i)\n", fd);
    
    return TRUE;
}


void WvDBus::remove_watch(DBusWatch *watch, void *data)
{
    WvDBus *stream = static_cast<WvDBus *>(data);

    if (stream == NULL)
        return;

    fprintf(stderr, "Watch removed (stream->fd: %i)\n", stream->getfd());

    stream->setfd(-1); // WLACH: uh.. is that right?
    stream->close(); // WLACH: uh.. is that right?
}


WvDBus::Err::Err()
    : log("DBus", WvLog::Error)
{
    dbus_error_init(this);
    reset();
}

WvDBus::Err::~Err()
{
    reset();
    dbus_error_free(this);
}

WvDBus::Err::Err *WvDBus::Err::reset(WvStringParm _why)
{
    print();
    why = _why;
    return this;
}

void WvDBus::Err::print()
{
    if (dbus_error_is_set(this))
    {
	log("%s: %s\n", why, message);
	dbus_error_free(this);
	dbus_error_init(this);
    }
}

void WvDBus::Msg::append(WvStringParm s1, WvStringParm s2, WvStringParm s3)
{
    const char *tmp;
    if (!s1.isnull())
    {
	tmp = s1.cstr();
	dbus_message_append_args(msg, DBUS_TYPE_STRING, &tmp,
				 DBUS_TYPE_INVALID);
    }
    else
	return;
    if (!s2.isnull())
    {
	tmp = s2.cstr();
	dbus_message_append_args(msg, DBUS_TYPE_STRING, &tmp,
				 DBUS_TYPE_INVALID);
    }
    else
	return;
    if (!s3.isnull())
    {
	tmp = s3.cstr();
	dbus_message_append_args(msg, DBUS_TYPE_STRING, &tmp,
				 DBUS_TYPE_INVALID);
    }
}

void WvDBus::Msg::decode(WvStringList &l) const
{
    DBusMessageIter iter;
    dbus_message_iter_init(msg, &iter);
    do
    {
	int type = dbus_message_iter_get_arg_type(&iter);
	switch (type)
	{
	case DBUS_TYPE_STRING:
	    {
		const char *str;
		dbus_message_iter_get_basic(&iter, &str);
		l.append(new WvString(str), true);
	    }
	    break;
	default:
	    // some other weird type
	    l.append(new WvString, true);
	    break;
	}
    } while (dbus_message_iter_next(&iter));
}

WvString WvDBus::Msg::arg(int n) const
{
    if (!msg)
	return WvString::null;

    if (args.isempty())
	decode(args);

    WvStringList::Iter i(args);
    for (i.rewind(); i.next(); )
    {
	if (--n < 0)
	    return i();
    }
    return WvString::null;
}

WvDBus::WvDBus(DBusBusType bus)
    : conn(NULL), userdata(NULL)
{
    conn = dbus_bus_get(bus, xerr("bus_get"));
    if (conn)
    {
        if (!dbus_connection_set_watch_functions(conn, add_watch, 
                                                 remove_watch, 
                                                 NULL,
/*                                             watch_toggled, */
                                                 this, NULL))
        {
            // set isok to false or something
        }
// 	dbus_connection_setup_with_g_main(conn, NULL);
	dbus_connection_add_filter(conn, filter_func, this, NULL);
    }
}

WvDBus::WvDBus(DBusConnection *c)
    : conn(c), userdata(NULL)
{
}

WvDBus::WvDBus(WvDBus &c)
    : conn(c), userdata(NULL)
{
    dbus_connection_ref(c);
}

WvDBus::~WvDBus()
{
    close();
}

void WvDBus::close()
{
    if (conn)
    {
	dbus_connection_flush(conn);
	dbus_connection_unref(conn);
	conn = NULL;
    }
}

void WvDBus::request_name(WvStringParm srv, int flag)
{
    if (conn)
	dbus_bus_request_name(conn, srv, flag, xerr("acquire"));
}

WvDBus::Msg WvDBus::sendsync(const WvDBus::Msg &msg, time_t msec_timeout)
{
    return WvDBus::Msg(
	dbus_connection_send_with_reply_and_block(conn, msg, msec_timeout,
						  xerr("waitreply")));
}

void WvDBus::send(const Msg &msg)
{
    if (conn)
	dbus_connection_send(conn, msg, NULL);
}

DBusHandlerResult WvDBus::filter_func(DBusConnection *_conn,
				      DBusMessage *_msg,
				      void *userdata)
{
    WvDBus *c = (WvDBus *)userdata;
    assert(*c == _conn);

    dbus_message_ref(_msg);
    WvDBus::Msg msg(_msg);

    if (c->callback)
	c->callback(*c, msg);

    return DBUS_HANDLER_RESULT_HANDLED;
}

void WvDBus::add_match(WvStringParm filter)
{
    dbus_bus_add_match(conn, filter, xerr("add_match"));
}

void WvDBus::set_userdata(void *d)
{
    userdata = d;
}

void *WvDBus::get_userdata() const
{
    return userdata;
}
