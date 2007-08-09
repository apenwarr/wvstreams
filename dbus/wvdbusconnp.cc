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
#include "wvdbusconnp.h"
#include "wvdbusconn.h"
#include "wvdbuswatch.h"

static DBusBusType bustypes[WvDBusConnBase::NUM_BUS_TYPES] 
    = { DBUS_BUS_SESSION, DBUS_BUS_SYSTEM, DBUS_BUS_STARTER };


static dbus_bool_t _add_watch(DBusWatch *watch, void *data)
{
    WvDBusConn *connp = (WvDBusConn *)data;
    return connp->add_watch(watch);
}


static void _remove_watch(DBusWatch *watch, void *data)
{
    WvDBusConn *connp = (WvDBusConn *)data;
    connp->remove_watch(watch);
}


static void _watch_toggled(DBusWatch *watch, void *data)
{
    WvDBusConn *connp = (WvDBusConn *)data;
    connp->watch_toggled(watch);
}


static dbus_bool_t _add_timeout(DBusTimeout *timeout, void *data)
{
    WvDBusConn *connp = (WvDBusConn *)data;
    return connp->add_timeout(timeout);
}


static void _remove_timeout(DBusTimeout *timeout, void *data)
{
    WvDBusConn *connp = (WvDBusConn *)data;
    connp->remove_timeout(timeout);
}


static void _timeout_toggled(DBusTimeout *timeout, void *data)
{
    WvDBusConn *connp = (WvDBusConn *)data;
    connp->timeout_toggled(timeout);
}


static DBusHandlerResult _filter_func(DBusConnection *_conn,
				      DBusMessage *_msg,
				      void *userdata)
{
    WvDBusConn *connp = (WvDBusConn *)userdata;
    return connp->filter_func(_conn, _msg);
}


WvDBusConn::WvDBusConn(DBusConnection *_c)
    : ifacedict(10)
{
    dbusconn = _c;
    dbus_connection_ref(dbusconn);
    init(false);
}


WvDBusConn::WvDBusConn(BusType bus)
    : ifacedict(10)
{
    DBusError error;
    dbus_error_init(&error);
    dbusconn = dbus_bus_get(bustypes[bus], &error);
    maybe_seterr(error);

    init(true);
}


WvDBusConn::WvDBusConn(WvStringParm dbus_moniker)
    : ifacedict(10)
{
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

    init(true);
}


WvDBusConn::~WvDBusConn()
{
    close();
    
    log("Releasing connection..\n");
    dbus_connection_unref(dbusconn);
    dbusconn = NULL;
}


void WvDBusConn::init(bool client)
{
    if (geterr())
	log(WvLog::Error, "Error in initialization: %s\n", errstr());
    
    assert(dbusconn);
    name_acquired = false;

    DBusWatchToggledFunction toggled_function = NULL;
    if (client)
        toggled_function = _watch_toggled;

    if (!dbus_connection_set_watch_functions(dbusconn, _add_watch,
                                             _remove_watch,
                                             toggled_function,
                                             this, NULL))
	seterr_both(EINVAL, "Couldn't set up watch functions\n");

    // FIXME: need to add real functions here, timeouts won't work until we
    // do
    dbus_connection_set_timeout_functions(dbusconn, _add_timeout,
                                          _remove_timeout,
                                          _timeout_toggled,
                                          this, NULL);

    log("Done init..\n");

    dbus_connection_add_filter(dbusconn, _filter_func, this, NULL);
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


void WvDBusConn::execute()
{
    // log("Execute.\n");
    WvDBusConnBase::execute();
    while (dbus_connection_dispatch(dbusconn) == 
           DBUS_DISPATCH_DATA_REMAINS);
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
    
    WvDBusConnBase::close();
}


dbus_bool_t WvDBusConn::add_watch(DBusWatch *watch)
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


dbus_bool_t WvDBusConn::add_timeout(DBusTimeout *timeout)
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


DBusHandlerResult WvDBusConn::filter_func(DBusConnection *_conn,
                                                 DBusMessage *_msg)
{
    print_message_trace(_msg);

    WvStringParm interface = dbus_message_get_interface(_msg);
    WvStringParm path = dbus_message_get_path(_msg);
    WvStringParm member = dbus_message_get_member(_msg);

    if (ifacedict[interface])
    {
        log(WvLog::Debug5, "Interface exists for message. Sending.\n");
        ifacedict[interface]->handle_signal(path, member, this, _msg);
        return DBUS_HANDLER_RESULT_HANDLED;
    }

    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}


void WvDBusConn::print_message_trace(DBusMessage *_msg)
{
    WvDBusMsg msg(_msg);
    log(WvLog::Debug5, "Rx: %s sig=%s\n", msg,
        dbus_message_is_signal(_msg, dbus_message_get_interface(_msg),
                               dbus_message_get_path(_msg)));
}


DBusConnection *WvDBusConn::_getconn() const
{
    return dbusconn;
}


