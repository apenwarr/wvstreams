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

static DBusBusType bustypes[WvDBusConn::NUM_BUS_TYPES] 
    = { DBUS_BUS_SESSION, DBUS_BUS_SYSTEM, DBUS_BUS_STARTER };


WvDBusConn::WvDBusConn(DBusConnection *_c, WvStringParm _uniquename)
    : log(WvString("DBus s%s", _uniquename)),
      ifacedict(10)
{
    log("Initializing.\n");
    assert(_c);
    dbusconn = _c;
    dbus_connection_ref(dbusconn);
    this->_uniquename = _uniquename;
    init(false);
}


WvDBusConn::WvDBusConn(BusType bus)
    : log(WvString("DBus #%s/%s", getpid(), ++conncount)),
      ifacedict(10)
{
    log("Initializing.\n");
    DBusError error;
    dbus_error_init(&error);
    dbusconn = dbus_bus_get(bustypes[bus], &error);
    maybe_seterr(error);

    _uniquename = dbus_bus_get_unique_name(dbusconn);
    
    init(true);
}


WvDBusConn::WvDBusConn(WvStringParm dbus_moniker)
    : log(WvString("DBus #%s/%s", getpid(), ++conncount)),
      ifacedict(10)
{
    log("Initializing.\n");
    assert(!!dbus_moniker);
    
    // it seems like we want to open a private connection to the bus
    // for this particular case.. we can't create multiple named connections
    // otherwise
    DBusError error;
    dbus_error_init(&error);
    dbusconn = dbus_connection_open_private(dbus_moniker, &error);
    maybe_seterr(error);

    // dbus_bus_get(..) does the following for us.. but we aren't using
    // dbus_bus_get
    //dbus_connection_set_exit_on_disconnect(dbusconn, FALSE);
    if (!dbus_bus_register(dbusconn, &error))
        log(WvLog::Error, "Error registering with the bus!\n");

    _uniquename = dbus_bus_get_unique_name(dbusconn);

    init(true);
}


WvDBusConn::~WvDBusConn()
{
    log("Shutting down.\n");
    if (geterr())
	log("Error was: %s\n", errstr());
    
    close();
    
    dbus_connection_unref(dbusconn);
    dbusconn = NULL;
}


void WvDBusConn::init(bool client)
{
    if (geterr())
	log(WvLog::Error, "Error in initialization: %s\n", errstr());
    
    assert(dbusconn);
    name_acquired = false;
    
    if (client)
	log("Server assigned name: '%s'\n", uniquename());
    
    if (!!_uniquename)
	log.app = WvString("DBus %s%s", client ? "c" : "s", uniquename());

    DBusWatchToggledFunction toggled_function = NULL;
    if (client)
        toggled_function = WvDBusConnHelpers::_watch_toggled;

    if (!dbus_connection_set_watch_functions(dbusconn,
				     WvDBusConnHelpers::_add_watch,
				     WvDBusConnHelpers::_remove_watch,
				     toggled_function,
				     this, NULL))
	seterr_both(EINVAL, "Couldn't set up watch functions\n");

    // FIXME: need to add real functions here, timeouts won't work until we
    // do
    dbus_connection_set_timeout_functions(dbusconn,
					  WvDBusConnHelpers::_add_timeout,
                                          WvDBusConnHelpers::_remove_timeout,
                                          WvDBusConnHelpers::_timeout_toggled,
                                          this, NULL);

    dbus_connection_add_filter(dbusconn, 
			       WvDBusConnHelpers::_filter_func,
			       this, NULL);

    log("Done init.\n");
}


void WvDBusConn::close()
{
    if (name_acquired)
    {
	DBusError error;
	dbus_error_init(&error);
	dbus_bus_release_name(dbusconn, name, &error);
	maybe_seterr(error);
    }
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


void WvDBusConn::request_name(WvStringParm name)
{
    assert(dbusconn);
    assert(!name_acquired);

    DBusError error;
    dbus_error_init(&error);
    
    this->name = name;

    int flags = (DBUS_NAME_FLAG_ALLOW_REPLACEMENT |
                 DBUS_NAME_FLAG_REPLACE_EXISTING);
    if (dbus_bus_request_name(dbusconn, name, flags, &error) == -1)
	maybe_seterr(error);
    else
    {
        log(WvLog::Debug5, "Set name '%s' for this connection.\n", name);
        name_acquired = true;
    }
}


bool WvDBusConn::isok() const
{
    return dbusconn && !geterr();
}


void WvDBusConn::execute()
{
    // log("Execute.\n");
    WvIStreamList::execute();
    while (dbus_connection_dispatch(dbusconn) == 
           DBUS_DISPATCH_DATA_REMAINS);
}


uint32_t WvDBusConn::send(WvDBusMsg &msg)
{
    uint32_t serial;
    log(WvLog::Debug, "Sending %s\n", msg);
    if (!dbus_connection_send(dbusconn, msg, &serial)) 
        seterr_both(ENOMEM, "Out of memory.\n");
    return serial;
}


void WvDBusConn::send(WvDBusMsg &msg, IWvDBusListener *reply, 
                      bool autofree_reply)
{
    log(WvLog::Debug, "Sending_w_r %s\n", msg);
    DBusPendingCall *pending;

    if (!dbus_connection_send_with_reply(dbusconn, msg, &pending, -1))
    { 
        seterr_both(ENOMEM, "Out of memory.\n");
        return;
    }

    DBusFreeFunction free_user_data = NULL;
    if (autofree_reply)
        free_user_data = &WvDBusConn::remove_listener_cb;

    if (!dbus_pending_call_set_notify(pending, 
                                      &WvDBusConn::pending_call_notify,
                                      reply, free_user_data))
        seterr_both(ENOMEM, "Out of memory.\n");
}


void WvDBusConn::add_callback(CallbackPri pri, WvDBusCallback cb, void *cookie)
{
#if 0
    DBusError error;
    dbus_error_init(&error);
    dbus_bus_add_match(dbusconn, "type='signal',interface='fork'", &error);
    maybe_seterr(error);
#endif
    
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


void WvDBusConn::add_listener(WvStringParm interface, WvStringParm path, 
                              IWvDBusListener *listener)
{
    DBusError error;
    dbus_error_init(&error);

    dbus_bus_add_match(dbusconn,
		       WvString("type='signal',interface='%s'", interface),
		       &error);
    maybe_seterr(error);
    if (isok())
	_add_listener(interface, path, listener);    
}


void WvDBusConn::del_listener(WvStringParm interface, WvStringParm path,
                              WvStringParm name)
{
    DBusError error;
    dbus_error_init(&error);

    dbus_bus_remove_match(dbusconn, 
                          WvString("type='signal',interface='%s'", interface),
                          &error);
    maybe_seterr(error);
    if (isok())
	_del_listener(interface, path, name);
}


void WvDBusConn::add_method(WvStringParm interface, WvStringParm path, 
                            IWvDBusListener *listener)
{
    _add_listener(interface, path, listener);
}


void WvDBusConn::del_method(WvStringParm interface, WvStringParm path,
                            WvStringParm name)
{
    _del_listener(interface, path, name);
}


void WvDBusConn::_add_listener(WvStringParm interface, 
                                       WvStringParm path,
                                       IWvDBusListener *listener)
{
    if (!ifacedict[interface])
        ifacedict.add(new WvDBusInterface(interface), true);
    ifacedict[interface]->add_listener(path, listener);
}


void WvDBusConn::_del_listener(WvStringParm interface, 
                                       WvStringParm path, WvStringParm name)
{
    if (!ifacedict[interface])
    {
        log(WvLog::Warning, "Attempted to delete listener with interface "
            "'%s', but interface does not exist! (path: %s name: %s)\n",
            path, name);
        return;
    }
    
}


void WvDBusConn::pending_call_notify(DBusPendingCall *pending, 
                                            void *user_data)
{
    IWvDBusListener *listener = (IWvDBusListener *)user_data;
    DBusMessage *msg = dbus_pending_call_steal_reply(pending);

    WvLog("pending", WvLog::Debug1).print("incoming: %s\n", WvDBusMsg(msg));
    listener->dispatch(msg);
    dbus_pending_call_unref(pending);
    dbus_message_unref(msg);
}


void WvDBusConn::remove_listener_cb(void *memory)
{
    IWvDBusListener *listener = (IWvDBusListener *)memory;
    delete listener;
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

    log(WvLog::Debug5, "Watch updated successfully (fd: %s, readable: %s, "
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
    log("Filter: %s\n", msg);
    
    // handle all the generic filters
    bool handled = false;
    CallbackInfoList::Sorter i(callbacks, priority_order);
    for (i.rewind(); i.next(); )
    {
	handled = i->cb(conn, msg) || handled; // || handled must be last!!
	log("Handled=%s\n", handled);
	if (handled) break;
    }

    // handle the fancy callbacks
    WvString ifc = msg.get_interface();
    if (ifacedict[ifc])
    {
        log(WvLog::Debug5, "Interface exists for message. Sending.\n");
        ifacedict[ifc]->handle_signal(msg.get_path(), msg.get_member(),
				      this, msg);
	handled = true;
    }

    return handled;
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


