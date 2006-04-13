/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2004-2006 Net Integration Technologies, Inc.
 * 
 */ 
#include "wvdbusconn.h"
#include "wvdbuswatch.h"


DeclareWvDict(WvDBusInterface, WvString, name);

class WvDBusConnPrivate
{
public:
    WvDBusConnPrivate(WvDBusConn *_conn, WvStringParm _name, DBusBusType bus) :
        conn(_conn),
        ifacedict(10),
        dbusconn(NULL),
        name(_name),
        log("WvDBusConnPrivate", WvLog::Debug)
    {
        DBusError error;
        dbus_error_init(&error);
        dbusconn = dbus_bus_get(bus, &error);
        
        init_dbusconn(true);
    }

    WvDBusConnPrivate(WvDBusConn *_conn, WvStringParm _name, 
                      WvStringParm _address) :
        conn(_conn),
        ifacedict(10),
        dbusconn(NULL),
        name(_name),
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

        init_dbusconn(true);
    }

    WvDBusConnPrivate(WvDBusConn *_conn, DBusConnection *_c) :
        conn(_conn),
        ifacedict(10),
        dbusconn(_c),
        name(""),
        log("WvDBusConnPrivate", WvLog::Debug)
    {
        dbus_connection_ref(dbusconn);
        init_dbusconn(false);
    }

    ~WvDBusConnPrivate()
     {
         DBusError error;
         dbus_error_init(&error);
         if (dbus_bus_release_name(dbusconn, name, &error) == (-1))
         {
             log(WvLog::Error, "Error releasing name '%s' for connection!\n",
                 name);
         }

         close();
     }

    void init_dbusconn(bool client)
    {
        if (dbusconn)
        {
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

            // FIXME: need to add this, timeouts won't work until we do
            dbus_connection_set_timeout_functions(dbusconn, add_timeout,
                                                  remove_timeout, 
                                                  timeout_toggled,
                                                  this, NULL);

            int flags = (DBUS_NAME_FLAG_ALLOW_REPLACEMENT | 
                         DBUS_NAME_FLAG_REPLACE_EXISTING);
            if (!!name && (dbus_bus_request_name (dbusconn, name, flags, 
                                                  &error) == (-1)))
            {
                log(WvLog::Error, "Couldn't set name '%s' for connection! "
                    "(error name: %s)\n", name, error.name);
                log(WvLog::Error, "Error message is: %s", error.message);
                // set isok to false or something
            }
            else
                log(WvLog::Debug5, "Set name '%s' for connection!\n",
                    name);

            log("Done..\n");

            if (client)
                dbus_connection_add_filter(dbusconn, filter_func, this, NULL);
        }
        
    }

    static dbus_bool_t add_watch(DBusWatch *watch, void *data)
    {
        WvDBusConnPrivate *connp = (WvDBusConnPrivate *)data;
        unsigned int flags = dbus_watch_get_flags(watch);

        WvDBusWatch *wwatch = new WvDBusWatch(watch, flags);
        connp->conn->append(wwatch, true);

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

    static void remove_watch(DBusWatch *watch, void *data)
    {
        WvDBusWatch *wwatch = (WvDBusWatch *)dbus_watch_get_data(watch);
        assert(wwatch);
//        fprintf(stderr, "Removing watch (stream->fd: %i)\n", wwatch->getfd());
//         wwatch->dissociate_fds();
        wwatch->close();
    }

    static void watch_toggled(DBusWatch *watch, void *data)
    {
        fprintf(stderr, "toggle watch\n");
        if (dbus_watch_get_enabled(watch))
            add_watch(watch, data);
        else
            remove_watch(watch, data);
    }

    static dbus_bool_t add_timeout(DBusTimeout *timeout, void *data)
    {
        fprintf(stderr, "Add timeout.\n");

        return TRUE;
    }

    static void remove_timeout(DBusTimeout *timeout, void *data)
    {
        fprintf(stderr, "Remove timeout.\n");
    }

    static void timeout_toggled(DBusTimeout *timeout, void *data)
    {
        fprintf(stderr, "Timeout toggled.\n");
    }

    static DBusHandlerResult filter_func(DBusConnection *_conn,
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


    static void pending_call_notify(DBusPendingCall *pending, void *user_data)
    {
        IWvDBusMarshaller * marshaller = (IWvDBusMarshaller *) user_data;
        DBusMessage *msg = dbus_pending_call_steal_reply(pending);

        marshaller->dispatch(msg);
    }


    static void remove_marshaller_cb(void *memory)
    {
        IWvDBusMarshaller * marshaller = (IWvDBusMarshaller *) memory;
        delete marshaller;
    }



    void execute()
    {
        log("Execute.\n");
        while (dbus_connection_dispatch(dbusconn) == DBUS_DISPATCH_DATA_REMAINS);
    }

    void close()
    {
        if (conn)
        {
            dbus_connection_flush(dbusconn);
            dbus_connection_unref(dbusconn);
            conn = NULL;
        }
    }

    WvDBusConn *conn;
    WvDBusInterfaceDict ifacedict;
    DBusConnection *dbusconn;
    WvString name;
    WvLog log;
};


WvDBusConn::WvDBusConn(WvStringParm name, DBusBusType bus)
    : log("WvDBusConn")
{
    log("Starting up..\n");
    priv = new WvDBusConnPrivate(this, name, bus);

}


WvDBusConn::WvDBusConn(WvStringParm name, WvStringParm address)
    : log("WvDBusConn")
{
    log("Starting up..\n");
    priv = new WvDBusConnPrivate(this, name, address);

}


WvDBusConn::WvDBusConn(DBusConnection *c)
    : log("WvDBusConn")
{
    priv = new WvDBusConnPrivate(this, c);
}


#if 0
WvDBusConn::WvDBusConn(WvDBusConn &c)
    : conn(c), log("WvDBusConn")
{
}
#endif

WvDBusConn::~WvDBusConn()
{
    WVDELETE(priv);
}


void WvDBusConn::execute()
{
    WvIStreamList::execute();
    priv->execute();
}


void WvDBusConn::close()
{
    priv->close();
}


void WvDBusConn::send(WvDBusMsg &msg)
{
    dbus_uint32_t serial = 0;

    if (!dbus_connection_send(priv->dbusconn, msg, &serial)) 
    { 
        log(WvLog::Error, "Out Of Memory!\n"); 
        // FIXME: what do we do NOW?
    }
    else
        log(WvLog::Debug, "DBus message sent with serial %s\n", serial);
}


void WvDBusConn::send(WvDBusMsg &msg, IWvDBusMarshaller *reply, bool autofree_reply)
{
    log(WvLog::Debug, "Sending message.\n");
    DBusPendingCall * pending;

    // FIXME: allow custom timeouts?
    if (!dbus_connection_send_with_reply(priv->dbusconn, msg, &pending, 1000)) 
    { 
        log(WvLog::Error, "Out Of Memory!\n"); 
        // FIXME: what do we do NOW?
        return;
    }

    if (pending == NULL) 
    { 
        log(WvLog::Error, "Pending Call Null\n"); 
        // FIXME: what do we do NOW?
    }

    DBusFreeFunction free_user_data = NULL;    
    if (autofree_reply)
        free_user_data = &WvDBusConnPrivate::remove_marshaller_cb;

    if (!dbus_pending_call_set_notify(pending, 
                                      &WvDBusConnPrivate::pending_call_notify,
                                      reply, free_user_data))
    {
        log(WvLog::Error, "Setting NOTIFY failed..\n"); 
        // FIXME: what do we do NOW?
    }

}


void WvDBusConn::add_marshaller(WvStringParm interface, WvStringParm path, 
                                IWvDBusMarshaller *marshaller)
{
    if (!priv->ifacedict[interface])
    {
        DBusError error;
        dbus_bus_add_match(priv->dbusconn, WvString("type='signal',interface='%s'", 
                                              interface),  &error);
        if (dbus_error_is_set(&error)) 
        { 
            log(WvLog::Error, "Oh no! Couldn't add a match on the bus!\n");
        }
        priv->ifacedict.add(new WvDBusInterface(interface), true);
    }

    priv->ifacedict[interface]->add_marshaller(path, marshaller);
}


void WvDBusConn::add_method(WvStringParm interface, WvStringParm path, 
                            IWvDBusMarshaller *listener)
{
    if (!priv->ifacedict[interface])
        priv->ifacedict.add(new WvDBusInterface(interface), true);

    priv->ifacedict[interface]->add_marshaller(path, listener);
}
