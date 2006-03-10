#include "wvdbusconn.h"


class WvDBusWatch : public WvFdStream
{
public:
    WvDBusWatch(DBusWatch *_watch, unsigned int flags);
    virtual void execute();
    // disable reading/writing: we want dbus to do that for us
    virtual size_t uread(void *buf, size_t count) { return 0; }
    virtual size_t uwrite(const void *buf, size_t count) { return 0; }
    DBusWatch *watch;
};


WvDBusWatch::WvDBusWatch(DBusWatch *_watch, unsigned int flags) :
    watch(_watch)
{
    int fd = dbus_watch_get_fd(watch);
    if (flags & DBUS_WATCH_READABLE)
        rfd = fd;
    else if (flags & DBUS_WATCH_WRITABLE)
        wfd = fd;
}


void WvDBusWatch::execute()
{
    unsigned int dbus_condition = 0;
//     fprintf(stderr, "Execute. %i %i\n", isreadable(), iswritable());

    if (isreadable())
        dbus_condition |= DBUS_WATCH_READABLE;
     if (iswritable())
         dbus_condition |= DBUS_WATCH_WRITABLE;
    // FIXME: Handle errors, HUP

    dbus_watch_handle(watch, dbus_condition);
}


DeclareWvDict(WvDBusInterface, WvString, name);

class WvDBusConnPrivate
{
public:
    WvDBusConnPrivate(WvDBusConn *_conn, DBusBusType bus) :
        conn(_conn),
        ifacedict(10),
        dbusconn(NULL),
        log("WvDBusConnPrivate")
    {
        DBusError error;
        dbusconn = dbus_bus_get(bus, &error);
        
        if (dbusconn)
        {
            log("Setting watch functions..\n");
            if (!dbus_connection_set_watch_functions(dbusconn, add_watch, 
                                                     remove_watch, 
                                                     watch_toggled,
                                                     this, NULL))
            {
                log(WvLog::Error, "Couldn't set up watch functions!\n");
                // set isok to false or something
            }
            log("Done..\n");

            dbus_connection_add_filter(dbusconn, filter_func, this, NULL);
        }        
    }

    ~WvDBusConnPrivate()
     {
         close();
     }

    static dbus_bool_t add_watch(DBusWatch *watch, void *data)
    {
        WvDBusConnPrivate *connp = (WvDBusConnPrivate *)data;

        unsigned int flags = dbus_watch_get_flags(watch);
        WvDBusWatch *wwatch = new WvDBusWatch(watch, flags);
        connp->conn->append(wwatch, true);

        // FIXME: do we need to explicitly say whether we are readable and writable?
        // (see below)
        bool isreadable = (flags & DBUS_WATCH_READABLE);
        bool iswritable = (flags & DBUS_WATCH_WRITABLE);

        fprintf(stderr, "Watch updated successfully (fd: %i, readable: %i, "
                "writable: %i)\n", dbus_watch_get_fd(watch), 
                isreadable, iswritable);
    
        return TRUE;
    }

    static void remove_watch(DBusWatch *watch, void *data)
    {
        WvDBusConnPrivate *connp = (WvDBusConnPrivate *)data;
        
        WvDBusWatch *wwatch = connp->get_watch(dbus_watch_get_fd(watch));
        fprintf(stderr, "Removing watch (stream->fd: %i)\n", wwatch->getfd());
        wwatch->close();
    }

    static void watch_toggled(DBusWatch *watch, void *data)
    {
        fprintf(stderr, "toggle watch\n");
    }

    WvDBusWatch * get_watch(int fd)
    {
        WvIStreamList::Iter i(*conn);
        for (i.rewind(); i.next();)
        {
            // FIXME: gross
            WvDBusWatch *wwatch = (WvDBusWatch *)&i();
            if (wwatch->getfd() == fd)
            {
                return wwatch;
            }
        }

        return NULL;
    }
 
    static DBusHandlerResult filter_func(DBusConnection *_conn,
                                         DBusMessage *_msg,
                                         void *userdata)
    {
        
        fprintf(stderr, "Got message. Sender %s. Path: %s. Interface: %s. Is signal: %i\n", 
                dbus_message_get_sender(_msg),
                dbus_message_get_path(_msg), 
                dbus_message_get_interface(_msg),
                dbus_message_is_signal(_msg, dbus_message_get_interface(_msg), 
                                       dbus_message_get_path(_msg)));

        WvDBusConnPrivate *priv = (WvDBusConnPrivate *)userdata;
        WvString ifacename = dbus_message_get_interface(_msg);
        WvStringParm path = dbus_message_get_path(_msg);

        if (priv->ifacedict[ifacename])
        {
            fprintf(stderr, "Interface exists for message. Sending.\n");
            priv->ifacedict[ifacename]->handle_signal(path, priv->conn, _msg);
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
    WvLog log;
};


WvDBusConn::WvDBusConn(DBusBusType bus)
    : log("WvDBusConn")
{
    log("Starting up..\n");
    priv = new WvDBusConnPrivate(this, bus);

}

#if 0
WvDBusConn::WvDBusConn(DBusConnection *c)
    : conn(c), log("WvDBusConn")
{
}


WvDBusConn::WvDBusConn(WvDBusConn &c)
    : conn(c), log("WvDBusConn")
{
    dbus_connection_ref(c);
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


void WvDBusConn::add_marshaller(WvStringParm ifacename, IWvDBusMarshaller *marshaller)
{
    if (!priv->ifacedict[ifacename])
    {
        DBusError error;
        dbus_bus_add_match(priv->dbusconn, WvString("type='signal',interface='%s'", 
                                              ifacename),  &error);
        if (dbus_error_is_set(&error)) 
        { 
            log(WvLog::Error, "Oh no! Couldn't add a match on the bus!\n");
        }
        priv->ifacedict.add(new WvDBusInterface(ifacename), true);
    }

    priv->ifacedict[ifacename]->d.add(marshaller, true);
}


void WvDBusConn::add_method(WvStringParm ifacename, IWvDBusMarshaller *listener)
{
    if (!priv->ifacedict[ifacename])
    {
        priv->ifacedict.add(new WvDBusInterface(ifacename), true);

        // request a name on the bus
        DBusError error;
        dbus_error_init(&error);
        
        const char *tmp = ifacename;

        int ret = dbus_bus_request_name(priv->dbusconn, "ca.nit.foo", 
                                        DBUS_NAME_FLAG_REPLACE_EXISTING, 
                                        &error);
        if (dbus_error_is_set(&error)) 
        { 
            log(WvLog::Error, "Name Error (%s)\n", error.message); 
            dbus_error_free(&error); 
        }
        
        if (DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != ret)
            log(WvLog::Error, "Oh no! DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != ret!\n");

        dbus_error_free(&error);
    }
    priv->ifacedict[ifacename]->d.add(listener, true);
}
