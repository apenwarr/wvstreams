#include "wvdbusconn.h"

class WvDBusConnPrivate
{

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


dbus_bool_t WvDBusConn::add_watch(DBusWatch *watch, void *data)     
{
    WvDBusConn *conn = (WvDBusConn *)data;

    unsigned int flags = dbus_watch_get_flags(watch);
    WvDBusWatch *wwatch = new WvDBusWatch(watch, flags);
    conn->append(wwatch, true);

    // FIXME: do we need to explicitly say whether we are readable and writable?
    // (see below)
    bool isreadable = (flags & DBUS_WATCH_READABLE);
    bool iswritable = (flags & DBUS_WATCH_WRITABLE);

    fprintf(stderr, "Watch updated successfully (fd: %i, readable: %i, "
            "writable: %i)\n", dbus_watch_get_fd(watch), 
            isreadable, iswritable);
    
    return TRUE;
}


void WvDBusConn::remove_watch(DBusWatch *watch, void *data)
{
    WvDBusConn *conn = (WvDBusConn *)data;

    WvDBusWatch *wwatch = conn->get_watch(dbus_watch_get_fd(watch));
    fprintf(stderr, "Removing watch (stream->fd: %i)\n", wwatch->getfd());
    wwatch->close();
}


static void watch_toggled(DBusWatch *watch, void *data)
{
    fprintf(stderr, "toggle watch\n");
}


DBusHandlerResult WvDBusConn::filter_func(DBusConnection *_conn,
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


WvDBusWatch * WvDBusConn::get_watch(int fd)
{
    WvIStreamList::Iter i(*this);
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


WvDBusConn::WvDBusConn(DBusBusType bus)
    : ifacedict(10), conn(NULL), log("WvDBusConn")
{
    log("Starting up..\n");
    DBusError error;
    conn = dbus_bus_get(bus, &error);

    if (conn)
    {
        log("Setting watch functions..\n");
        if (!dbus_connection_set_watch_functions(conn, add_watch, 
                                                 remove_watch, 
                                                 watch_toggled,
                                                 this, NULL))
        {
            // set isok to false or something
        }
        log("Done..\n");
        dbus_bus_add_match(conn, "type='signal',interface='ca.nit.foo'",  &error);
        dbus_connection_add_filter(conn, filter_func, this, NULL);
    }
}


WvDBusConn::WvDBusConn(DBusConnection *c)
    : ifacedict(10), conn(c), log("WvDBusConn")
{
}


WvDBusConn::WvDBusConn(WvDBusConn &c)
    : ifacedict(10), conn(c), log("WvDBusConn")
{
    dbus_connection_ref(c);
}


WvDBusConn::~WvDBusConn()
{
    close();
}


void WvDBusConn::execute()
{
    WvIStreamList::execute();
    while (dbus_connection_dispatch(conn) == DBUS_DISPATCH_DATA_REMAINS);

//     DBusDispatchStatus status = 
//         dbus_connection_dispatch(conn); // FIXME: returns status.. (but glib doesn't
//                                         // do anything with it either..)
}


void WvDBusConn::close()
{
    if (conn)
    {
        dbus_connection_flush(conn);
        dbus_connection_unref(conn);
        conn = NULL;
    }
}
