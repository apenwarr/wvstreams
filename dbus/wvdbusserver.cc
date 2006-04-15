/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2005-2006 Net Integration Technologies, Inc.
 * 
 */ 
#include "wvdbusservconn.h"
#include "wvdbusserver.h"
#include "wvdbuswatch.h"


class WvDBusServerPrivate 
{
public:
    WvDBusServerPrivate(WvStringParm addr, WvDBusServer *_server) :
        log("WvDBusServerPrivate", WvLog::Debug)
    {
        server = _server;

        DBusError error;
        dbus_error_init(&error);
        dbusserver = dbus_server_listen(addr, &error);

        if (!dbusserver)
        {
            log(WvLog::Error, "Couldn't create server!\n");
            return;
        }

        dbus_server_set_new_connection_function(dbusserver, new_connection_cb,
                                                server, NULL);

        if (!dbus_server_set_watch_functions(dbusserver, add_watch, 
                                             remove_watch,
                                             watch_toggled,
                                             this, NULL))
        {
            log(WvLog::Error, "Couldn't set up watch functions!\n");
            // set isok to false or something
        }
        
        // FIXME: need to add this, timeouts won't work until we do
        dbus_server_set_timeout_functions(dbusserver, add_timeout,
                                          remove_timeout, timeout_toggled,
                                          this, NULL);

        char * final_addr = dbus_server_get_address(dbusserver);
        log(WvLog::Info, "Final address of server is '%s'", final_addr);
        free(final_addr);
    }

    bool isok()
    {
        return (dbusserver != NULL);
    }

    void execute()
    {
    }

    static dbus_bool_t add_watch(DBusWatch *watch, void *data)
    {
        WvDBusServerPrivate *serverp = (WvDBusServerPrivate *)data;

        unsigned int flags = dbus_watch_get_flags(watch);
        WvDBusWatch *wwatch = new WvDBusWatch(watch, flags);
        serverp->server->append(wwatch, true, "wvdbuswatch");

        dbus_watch_set_data(watch, wwatch, NULL);

        // FIXME: do we need to explicitly say whether we are readable and 
        // writable? (see below)
        bool isreadable = (flags & DBUS_WATCH_READABLE);
        bool iswritable = (flags & DBUS_WATCH_WRITABLE);

        fprintf(stderr, "Watch updated successfully (fd: %i, readable: %i, "
                "writable: %i data: %p)\n", dbus_watch_get_fd(watch), 
                isreadable, iswritable, data);
    
        return TRUE;
    }

    static void remove_watch(DBusWatch *watch, void *data)
    {
        WvDBusWatch *wwatch = (WvDBusWatch *)dbus_watch_get_data(watch);
        assert(wwatch);
        fprintf(stderr, "Removing watch (rfd: %i wfd: $i)\n", wwatch->getrfd(),
                wwatch->getwfd());
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

    static void new_connection_cb(DBusServer *dbusserver, 
                                  DBusConnection *new_connection,
                                  void *userdata)
    {
        WvDBusServer *server = (WvDBusServer *) userdata;
        WvDBusServConn *c = new WvDBusServConn(new_connection);
        fprintf(stderr, "New connection..\n");
        server->append(c, true, "wvdbus connection");

        if (dbus_connection_get_dispatch_status(new_connection) != 
            DBUS_DISPATCH_COMPLETE)
        {
            dbus_connection_dispatch(new_connection);
        }
    }

    WvDBusServer *server;
    DBusServer *dbusserver;
    WvLog log;
};


WvDBusServer::WvDBusServer(WvStringParm addr) :
    log("WvDBusServer", WvLog::Debug)
{
    priv = new WvDBusServerPrivate(addr, this);    
}


WvDBusServer::~WvDBusServer()
{
    WVDELETE(priv);
}


void WvDBusServer::execute()
{
    WvIStreamList::execute();
    priv->execute();
}
