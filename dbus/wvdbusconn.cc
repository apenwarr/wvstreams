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
#include "wvdbusconn.h"
#include "wvdbuswatch.h"
#include <dbus/dbus.h>

class WvDBusConnHelpers
{
public:
    static dbus_bool_t _add_watch(DBusWatch *watch, void *data);
    static void _remove_watch(DBusWatch *watch, void *data);
    static void _watch_toggled(DBusWatch *watch, void *data);
    static dbus_bool_t _add_timeout(DBusTimeout *timeout, void *data);
    static void _remove_timeout(DBusTimeout *timeout, void *data);
    static void _timeout_toggled(DBusTimeout *timeout, void *data);
    static DBusHandlerResult _filter_func(DBusConnection *_conn,
					  DBusMessage *_msg,
					  void *userdata);
};

static int conncount;

WvDBusConn::WvDBusConn(DBusConnection *_c, WvStringParm _uniquename)
    : log(WvString("DBus s%s", _uniquename), WvLog::Debug4)
{
    log("Initializing.\n");
    assert(_c);
    dbusconn = _c;
    if (dbusconn)
	dbus_connection_ref(dbusconn);
    this->_uniquename = _uniquename;
    registered = true; // no need to register with server: we are the server!
    init(false);
}


WvDBusConn::WvDBusConn(WvStringParm dbus_moniker)
    : log(WvString("DBus #%s/%s", getpid(), ++conncount), WvLog::Debug4)
{
    log("Initializing.\n");
    assert(!!dbus_moniker);
    
    // it seems like we want to open a private connection to the bus
    // for this particular case.. we can't create multiple named connections
    // otherwise
    DBusError error;
    dbus_error_init(&error);
    dbusconn = NULL;
    if (!strncasecmp(dbus_moniker, "bus:", 4))
    {
	WvString busname = dbus_moniker+4;
	if (!strcasecmp(busname, "system"))
	    dbusconn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &error);
	else if (!strcasecmp(busname, "session"))
	    dbusconn = dbus_bus_get_private(DBUS_BUS_SESSION, &error);
	else if (!strcasecmp(busname, "starter"))
	    dbusconn = dbus_bus_get_private(DBUS_BUS_STARTER, &error);
	else
	    seterr("No such bus '%s'", busname);
	registered = true; // dbus_bus_get_* is supposed to do this for us
    }
    else
    {
	dbusconn = dbus_connection_open_private(dbus_moniker, &error);
	registered = false;
    }
    maybe_seterr(error);

    init(true);
}


WvDBusConn::~WvDBusConn()
{
    log("Shutting down.\n");
    if (geterr())
	log("Error was: %s\n", errstr());
    
    close();
    
    if (dbusconn)
    {
	execute(); // flush all "timed out due to close" messages
	dbus_connection_unref(dbusconn);
    }
}


void WvDBusConn::init(bool client)
{
    if (client && isok())
    {
	DBusError error;
	dbus_error_init(&error);
	if (!registered)
	    dbus_bus_register(dbusconn, &error);
	maybe_seterr(error);
    
	if (isok())
	{
	    _uniquename = dbus_bus_get_unique_name(dbusconn);
	    log("Server assigned name: '%s'\n", uniquename());
	    registered = true;
	}
    }

    DBusWatchToggledFunction toggled_function = NULL;
    if (client)
        toggled_function = WvDBusConnHelpers::_watch_toggled;

    if (isok())
    {
	if (!dbus_connection_set_watch_functions(dbusconn,
				     WvDBusConnHelpers::_add_watch,
				     WvDBusConnHelpers::_remove_watch,
				     toggled_function,
				     this, NULL))
	    seterr_both(EINVAL, "Couldn't set up watch functions\n");

	// FIXME: need to add real functions here, timeouts won't work until
	// we do
	dbus_connection_set_timeout_functions(dbusconn,
				      WvDBusConnHelpers::_add_timeout,
				      WvDBusConnHelpers::_remove_timeout,
				      WvDBusConnHelpers::_timeout_toggled,
				      this, NULL);

	dbus_connection_add_filter(dbusconn, 
				   WvDBusConnHelpers::_filter_func,
				   this, NULL);
    }
    
    if (!!_uniquename)
	log.app = WvString("DBus %s%s", client ? "c" : "s", uniquename());

    if (client && isok())
    {
	DBusError error;
	dbus_error_init(&error);
	dbus_bus_add_match(dbusconn, "type='signal'", &error);
	maybe_seterr(error);
    }
    
    if (geterr())
	log(WvLog::Error, "Error in initialization: %s\n", errstr());
}


void WvDBusConn::close()
{
    if (dbusconn)
	dbus_connection_close(dbusconn);
    
    WvIStreamList::close();
    assert(!isok());
}


void WvDBusConn::maybe_seterr(DBusError &e)
{
    if (dbus_error_is_set(&e))
	seterr_both(EIO, "%s: %s", e.name, e.message);
}


WvString WvDBusConn::uniquename() const
{
    return _uniquename;
}


void WvDBusConn::request_name(WvStringParm name, WvDBusCallback onreply)
{
    if (!isok()) return;

    uint32_t flags = (DBUS_NAME_FLAG_ALLOW_REPLACEMENT |
                 DBUS_NAME_FLAG_REPLACE_EXISTING);
    WvDBusMsg msg("org.freedesktop.DBus", "/org/freedesktop/DBus",
	      "org.freedesktop.DBus", "RequestName");
    msg.append(name).append(flags);
    if (!!onreply)
	send(msg, onreply);
    else
	send(msg);
    log("Requested name '%s' for this connection.\n", name);
}


bool WvDBusConn::isok() const
{
    return dbusconn && WvIStreamList::isok() && !geterr();
}


void WvDBusConn::execute()
{
    // log("Execute.\n");
    WvIStreamList::execute();
    if (!dbusconn) return;
    while (dbus_connection_dispatch(dbusconn) == DBUS_DISPATCH_DATA_REMAINS)
	;
}


uint32_t WvDBusConn::send(WvDBusMsg &msg)
{
    if (!isok()) return 0;
    uint32_t serial;
    log("Tx#%s: %s(%s)\n",
	msg.get_serial(), msg.get_member(), msg.get_argstr());
    if (!dbus_connection_send(dbusconn, msg, &serial)) 
        seterr_both(ENOMEM, "Out of memory.\n");
    return serial;
}


struct WvDBusUserData
{
    WvDBusConn *conn;
    WvDBusCallback cb;
    
    WvDBusUserData(WvDBusConn *_conn, const WvDBusCallback &_cb)
	: cb(_cb)
    {
	conn = _conn;
    }
};


static void free_callback(void *memory)
{
    WvDBusUserData *ud = (WvDBusUserData *)memory;
    delete ud;
}


void WvDBusConn::pending_call_notify(DBusPendingCall *pending, 
				     void *user_data)
{
    WvDBusUserData *ud = (WvDBusUserData *)user_data;
    DBusMessage *_msg = dbus_pending_call_steal_reply(pending);
    WvDBusMsg msg(_msg);

    bool handled = ud->cb(*ud->conn, msg);
    assert(handled); // reply function should *always* handle the reply
    dbus_pending_call_unref(pending);
    dbus_connection_unref(ud->conn->dbusconn);
    dbus_message_unref(msg);
}


void WvDBusConn::send(WvDBusMsg &msg, const WvDBusCallback &onreply) 
{
    if (!isok()) return;
    log("Tx_w_r %s\n", msg);
    DBusPendingCall *pending;

    if (!dbus_connection_send_with_reply(dbusconn, msg, &pending, -1)
	|| !pending)
    { 
        seterr_both(ENOMEM, "Out of memory.\n");
        return;
    }

    if (!dbus_pending_call_set_notify(pending, 
                                      &WvDBusConn::pending_call_notify,
				      new WvDBusUserData(this, onreply),
                                      free_callback))
    {
	seterr_both(ENOMEM, "Out of memory.\n");
	return;
    }
    
    dbus_connection_ref(dbusconn); // needed by the pending call
}


void WvDBusConn::add_callback(CallbackPri pri, WvDBusCallback cb, void *cookie)
{
    callbacks.append(new CallbackInfo(pri, cb, cookie), true);
}


void WvDBusConn::del_callback(void *cookie)
{
    // remember, there might be more than one callback with the same cookie.
    CallbackInfoList::Iter i(callbacks);
    for (i.rewind(); i.next(); )
	if (i->cookie == cookie)
	    i.xunlink();
}


bool WvDBusConn::add_watch(DBusWatch *watch)
{
    unsigned int flags = dbus_watch_get_flags(watch);

    WvDBusWatch *wwatch = new WvDBusWatch(watch, flags);
    append(wwatch, true, "D-Bus watch");

    dbus_watch_set_data(watch, wwatch, NULL);
#if 0
    // FIXME: do we need to explicitly say whether we are readable and
    // writable? (see below)
    bool isreadable = (flags & DBUS_WATCH_READABLE);
    bool iswritable = (flags & DBUS_WATCH_WRITABLE);

    log("Watch updated successfully (fd: %s, readable: %s, "
        "writable: %s)\n", dbus_watch_get_fd(watch),
        isreadable, iswritable);
#endif
    return TRUE;
}


void WvDBusConn::remove_watch(DBusWatch *watch)
{
    WvDBusWatch *wwatch = (WvDBusWatch *)dbus_watch_get_data(watch);
    assert(wwatch);

//    log(WvLog::Debug5, "Removing watch (rfd: %s wfd: %s)\n", 
//        wwatch->getrfd(), wwatch->getwfd());
    wwatch->close();
}


void WvDBusConn::watch_toggled(DBusWatch *watch)
{
//    log(WvLog::Debug5, "toggle watch\n");
    if (!watch)
        return;

    if (dbus_watch_get_enabled(watch))
        add_watch(watch);
    else
        remove_watch(watch);
}


bool WvDBusConn::add_timeout(DBusTimeout *timeout)
{
//    log(WvLog::Debug5, "Add timeout.\n");
    return TRUE;
}


void WvDBusConn::remove_timeout(DBusTimeout *timeout)
{
//    log(WvLog::Debug5, "Remove timeout.\n");
}


void WvDBusConn::timeout_toggled(DBusTimeout *timeout)
{
//    log(WvLog::Debug5, "Timeout toggled.\n");
}


int WvDBusConn::priority_order(const CallbackInfo *a, const CallbackInfo *b)
{
    return a->pri - b->pri;
}


bool WvDBusConn::filter_func(WvDBusConn &conn, WvDBusMsg &msg)
{
    log("Rx#%s: %s\n", msg.get_serial(), msg);
    
    if (msg.get_path() == "/org/freedesktop/DBus/Local")
    {
	if (msg.get_member() == "Disconnected")
	{
	    close();
	    return true;
	}
    }
    
    // handle all the generic filters
    CallbackInfoList::Sorter i(callbacks, priority_order);
    for (i.rewind(); i.next(); )
    {
	bool handled = i->cb(conn, msg);
	if (handled) return true;
    }

    return false; // couldn't handle the message, sorry
}


dbus_bool_t WvDBusConnHelpers::_add_watch(DBusWatch *watch, void *data)
{
    WvDBusConn *connp = (WvDBusConn *)data;
    return connp->add_watch(watch);
}


void WvDBusConnHelpers::_remove_watch(DBusWatch *watch, void *data)
{
    WvDBusConn *connp = (WvDBusConn *)data;
    connp->remove_watch(watch);
}


void WvDBusConnHelpers::_watch_toggled(DBusWatch *watch, void *data)
{
    WvDBusConn *connp = (WvDBusConn *)data;
    connp->watch_toggled(watch);
}


dbus_bool_t WvDBusConnHelpers::_add_timeout(DBusTimeout *timeout, void *data)
{
    WvDBusConn *connp = (WvDBusConn *)data;
    return connp->add_timeout(timeout);
}


void WvDBusConnHelpers::_remove_timeout(DBusTimeout *timeout, void *data)
{
    WvDBusConn *connp = (WvDBusConn *)data;
    connp->remove_timeout(timeout);
}


void WvDBusConnHelpers::_timeout_toggled(DBusTimeout *timeout, void *data)
{
    WvDBusConn *connp = (WvDBusConn *)data;
    connp->timeout_toggled(timeout);
}


DBusHandlerResult WvDBusConnHelpers::_filter_func(DBusConnection *_conn,
						  DBusMessage *_msg,
						  void *userdata)
{
    WvDBusConn *conn = (WvDBusConn *)userdata;
    WvDBusMsg msg(_msg);
    return conn->filter_func(*conn, msg) 
	? DBUS_HANDLER_RESULT_HANDLED : DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}
