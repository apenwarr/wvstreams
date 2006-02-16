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


class WvDBusConnPrivate
{
public:
    WvDBusConnPrivate(WvDBusConn *_conn, DBusBusType bus) :
        conn(_conn),
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
                // set isok to false or something
            }
            log("Done..\n");
            dbus_bus_add_match(dbusconn, "type='signal',interface='ca.nit.foo'",  &error);
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
#if 0
        WvDBusConn *c = (WvDBusConn *)userdata;
        assert(*c == _conn);
        
        dbus_message_ref(_msg);
        WvDBusMsg msg(_msg);
        
        if (c->callback)
            c->callback(*c, msg);
#endif
        
        return DBUS_HANDLER_RESULT_HANDLED;
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
    DBusConnection *dbusconn;
    WvLog log;
};


WvDBusConn::WvDBusConn(DBusBusType bus)
    : ifacedict(10), log("WvDBusConn")
{
    log("Starting up..\n");
    priv = new WvDBusConnPrivate(this, bus);

}

#if 0
WvDBusConn::WvDBusConn(DBusConnection *c)
    : ifacedict(10), conn(c), log("WvDBusConn")
{
}


WvDBusConn::WvDBusConn(WvDBusConn &c)
    : ifacedict(10), conn(c), log("WvDBusConn")
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
