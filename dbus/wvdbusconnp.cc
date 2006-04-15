/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2004-2006 Net Integration Technologies, Inc.
 * 
 */ 
#include "wvdbusconnp.h"
#include "wvdbusconn.h"
#include "wvdbuswatch.h"


WvDBusConnPrivate::WvDBusConnPrivate(WvDBusConn *_conn, WvStringParm _name, 
                                     DBusBusType bus) :
    conn(_conn),
    ifacedict(10),
    dbusconn(NULL),
    name(_name),
    name_acquired(false),
    log("WvDBusConnPrivate", WvLog::Debug)
{
    DBusError error;
    dbus_error_init(&error);
    dbusconn = dbus_bus_get(bus, &error);

    init(true);
    request_name(name);
}


WvDBusConnPrivate::WvDBusConnPrivate(WvDBusConn *_conn, WvStringParm _name,
                                     WvStringParm _address) :
    conn(_conn),
    ifacedict(10),
    dbusconn(NULL),
    name(_name),
    name_acquired(false),
    log("WvDBusConnPrivate", WvLog::Debug)
{
    DBusError error;
    dbus_error_init(&error);
    dbusconn = dbus_connection_open(_address, &error);
    if (dbus_error_is_set(&error))
    {
        log(WvLog::Error, "bad news! dbus error is set! %s %s\n",
            error.name, error.message);
    }

    // dbus_bus_get(..) does the following for us.. but we aren't using
    // dbus_bus_get
    dbus_connection_set_exit_on_disconnect(dbusconn, TRUE);
    if (!dbus_bus_register(dbusconn, &error))
    {
        log(WvLog::Error, "Error registering with the bus!\n");
    }

    init(true);
    request_name(name);
}


WvDBusConnPrivate::WvDBusConnPrivate(WvDBusConn *_conn, DBusConnection *_c) :
    conn(_conn),
    ifacedict(10),
    dbusconn(_c),
    name(""),
    name_acquired(false),
    log("WvDBusConnPrivate", WvLog::Debug)
{
    dbus_connection_ref(dbusconn);
    init(false);
}


WvDBusConnPrivate::~WvDBusConnPrivate()
{
    if (name_acquired)
    {
        DBusError error;
        dbus_error_init(&error);
        if (dbus_bus_release_name(dbusconn, name, &error) == (-1))
        {
            log(WvLog::Error, "Error releasing name '%s' for connection!\n",
                name);
        }
    }

    close();
}


void WvDBusConnPrivate::init(bool client)
{
    assert(dbusconn);

    DBusError error;
    dbus_error_init(&error);

    DBusWatchToggledFunction toggled_function = NULL;
    if (client)
        toggled_function = watch_toggled;

    if (!dbus_connection_set_watch_functions(dbusconn, add_watch,
                                             remove_watch,
                                             toggled_function,
                                             this, NULL))
    {
        log(WvLog::Error, "Couldn't set up watch functions!\n");
        // set isok to false or something
    }

    // FIXME: need to add real functions here, timeouts won't work until we
    // do
    dbus_connection_set_timeout_functions(dbusconn, add_timeout,
                                          remove_timeout,
                                          timeout_toggled,
                                          this, NULL);

    log("Done init..\n");

    dbus_connection_add_filter(dbusconn, filter_func, this, NULL);
}


void WvDBusConnPrivate::request_name(WvStringParm name)
{
    assert(dbusconn);

    DBusError error;
    dbus_error_init(&error);

    int flags = (DBUS_NAME_FLAG_ALLOW_REPLACEMENT |
                 DBUS_NAME_FLAG_REPLACE_EXISTING);
    if (dbus_bus_request_name(dbusconn, name, flags, &error) == (-1))
    {
        log(WvLog::Error, "Couldn't set name '%s' for connection! "
            "(error name: %s)\n", name, error.name);
        log(WvLog::Error, "Error message is: %s", error.message);
        // set isok to false or something
    }
    else
    {
        log(WvLog::Debug5, "Set name '%s' for connection!\n",
            name);
        name_acquired = true;
    }
}


void WvDBusConnPrivate::add_marshaller(WvStringParm interface, 
                                       WvStringParm path,
                                       IWvDBusMarshaller *marshaller)
{
    if (!ifacedict[interface])
    {
        ifacedict.add(new WvDBusInterface(interface), true);
    }

    ifacedict[interface]->add_marshaller(path, marshaller);
}


dbus_bool_t WvDBusConnPrivate::add_watch(DBusWatch *watch, void *data)
{
    WvDBusConnPrivate *connp = (WvDBusConnPrivate *)data;
    unsigned int flags = dbus_watch_get_flags(watch);

    WvDBusWatch *wwatch = new WvDBusWatch(watch, flags);
    connp->conn->append(wwatch, true, "D-Bus watch");

    dbus_watch_set_data(watch, wwatch, NULL);

    // FIXME: do we need to explicitly say whether we are readable and
    // writable? (see below)
    bool isreadable = (flags & DBUS_WATCH_READABLE);
    bool iswritable = (flags & DBUS_WATCH_WRITABLE);

    fprintf(stderr, "Watch updated successfully (fd: %i, readable: %i, "
            "writable: %i)\n", dbus_watch_get_fd(watch),
            isreadable, iswritable);

    return TRUE;
}


void WvDBusConnPrivate::remove_watch(DBusWatch *watch, void *data)
{
    WvDBusWatch *wwatch = (WvDBusWatch *)dbus_watch_get_data(watch);
    assert(wwatch);

    fprintf(stderr, "Removing watch (rfd: %i wfd: %i)\n", wwatch->getrfd(), 
            wwatch->getwfd());
    wwatch->close();
}


void WvDBusConnPrivate::watch_toggled(DBusWatch *watch, void *data)
{
    fprintf(stderr, "toggle watch\n");
    if (!watch)
        return;

    if (dbus_watch_get_enabled(watch))
        add_watch(watch, data);
    else
        remove_watch(watch, data);
}


dbus_bool_t WvDBusConnPrivate::add_timeout(DBusTimeout *timeout, 
                                           void *data)
{
    fprintf(stderr, "Add timeout.\n");

    return TRUE;
}


void WvDBusConnPrivate::remove_timeout(DBusTimeout *timeout, void *data)
{
    fprintf(stderr, "Remove timeout.\n");
}


void WvDBusConnPrivate::timeout_toggled(DBusTimeout *timeout, 
                                        void *data)
{
    fprintf(stderr, "Timeout toggled.\n");
}


DBusHandlerResult WvDBusConnPrivate::filter_func(DBusConnection *_conn,
                                                 DBusMessage *_msg,
                                                 void *userdata)
{

    fprintf(stderr, "Got message. Sender %s. Destination: %s. Path: %s. "
            "Interface: %s. Member: %s. Is signal: %i\n",
            dbus_message_get_sender(_msg),
            dbus_message_get_destination(_msg),
            dbus_message_get_path(_msg),
            dbus_message_get_interface(_msg),
            dbus_message_get_member(_msg),
            dbus_message_is_signal(_msg, dbus_message_get_interface(_msg),
                                   dbus_message_get_path(_msg)));

    WvDBusConnPrivate *priv = (WvDBusConnPrivate *)userdata;
    WvStringParm interface = dbus_message_get_interface(_msg);
    WvStringParm path = dbus_message_get_path(_msg);
    WvStringParm member = dbus_message_get_member(_msg);

    if (priv->ifacedict[interface])
    {
        fprintf(stderr, "Interface exists for message. Sending.\n");
        priv->ifacedict[interface]->handle_signal(path, member,
                                                  priv->conn, _msg);
    }

    return DBUS_HANDLER_RESULT_HANDLED;
}


void WvDBusConnPrivate::pending_call_notify(DBusPendingCall *pending, 
                                            void *user_data)
{
    IWvDBusMarshaller * marshaller = (IWvDBusMarshaller *) user_data;
    DBusMessage *msg = dbus_pending_call_steal_reply(pending);

    marshaller->dispatch(msg);
}


void WvDBusConnPrivate::remove_marshaller_cb(void *memory)
{
    IWvDBusMarshaller * marshaller = (IWvDBusMarshaller *) memory;
    delete marshaller;
}


void WvDBusConnPrivate::execute()
{
    log("Execute.\n");
    while (dbus_connection_dispatch(dbusconn) == DBUS_DISPATCH_DATA_REMAINS);
}


void WvDBusConnPrivate::close()
{
    if (conn)
    {
        dbus_connection_flush(dbusconn);
        dbus_connection_unref(dbusconn);
        conn = NULL;
    }
}
