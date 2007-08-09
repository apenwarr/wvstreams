/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2005-2006 Net Integration Technologies, Inc.
 * 
 * Pathfinder Software:
 *   Copyright (C) 2007, Carillon Information Security Inc.
 *
 * This library is licensed under the LGPL, please read LICENSE for details.
 *
 */ 
#include "wvdbusservconn.h"
#include "wvdbusserver.h"
#include "wvdbuswatch.h"
#include <dbus/dbus.h>

#if 0
#define LOG_TRACE(info...) fprintf(stderr, info)
#else
#define LOG_TRACE(info...)
#endif

class WvDBusServerPrivate 
{
public:
    WvDBusServerPrivate(WvStringParm addr, WvDBusServer *_server) :
        log("DBus Server", WvLog::Debug)
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

        log(WvLog::Info, "Listening on '%s'\n", get_addr());
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

        LOG_TRACE("Watch updated successfully (fd: %i, readable: %i, "
                  "writable: %i)\n", dbus_watch_get_fd(watch),
                  (flags & DBUS_WATCH_READABLE), 
                  (flags & DBUS_WATCH_WRITABLE));
    
        return TRUE;
    }

    static void remove_watch(DBusWatch *watch, void *data)
    {
        WvDBusWatch *wwatch = (WvDBusWatch *)dbus_watch_get_data(watch);
        assert(wwatch);
        LOG_TRACE("Removing watch (rfd: %i wfd: %i)\n", wwatch->getrfd(),
                  wwatch->getwfd());
        wwatch->close();
    }

    static void watch_toggled(DBusWatch *watch, void *data)
    {
        LOG_TRACE("toggle watch\n");
        if (dbus_watch_get_enabled(watch))
            add_watch(watch, data);
        else
            remove_watch(watch, data);
    }

    static dbus_bool_t add_timeout(DBusTimeout *timeout, void *data)
    {
        LOG_TRACE("Add timeout.\n");

        return TRUE;
    }

    static void remove_timeout(DBusTimeout *timeout, void *data)
    {
        LOG_TRACE("Remove timeout.\n");
    }

    static void timeout_toggled(DBusTimeout *timeout, void *data)
    {
        LOG_TRACE("Timeout toggled.\n");
    }

    static void new_connection_cb(DBusServer *dbusserver, 
                                  DBusConnection *new_connection,
                                  void *userdata)
    {
        WvDBusServer *server = (WvDBusServer *) userdata;
        WvDBusServConn *c = new WvDBusServConn(new_connection, server);
        LOG_TRACE("New connection..\n");
        server->append(c, true, "wvdbus connection");

        if (dbus_connection_get_dispatch_status(new_connection) != 
            DBUS_DISPATCH_COMPLETE)
        {
            dbus_connection_dispatch(new_connection);
        }
    }

    WvString get_addr()
    {
        char * final_addr = dbus_server_get_address(dbusserver);
        WvString faddr("%s", final_addr);
        free(final_addr);
        
        return faddr;
    }

    WvDBusServer *server;
    DBusServer *dbusserver;
    WvLog log;
};


WvDBusServer::WvDBusServer(WvStringParm addr) :
    cdict(10),
    rsdict(10),
    log("DBus Server", WvLog::Debug)
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


void WvDBusServer::register_conn(WvDBusConn *conn)
{
    assert(!cdict[conn->name]);
    cdict.add(conn, false);
}


void WvDBusServer::proxy_msg(WvStringParm name, WvDBusConn *src, 
                             WvDBusMsg &msg)
{
    log("Proxying %s -> %s\n", msg, name);
    WvDBusConn *conn = cdict[name];
    uint32_t serial;
    if (conn)
    {
        serial = conn->send(msg);
        rsdict.add(new WvDBusReplySerial(serial, src), true);
        log("Proxy: now expecting reply #%s to %s\n",
            serial, src->name);
    }
    else
        log(WvLog::Warning,
	    "Proxy: no connection for '%s'\n", name);
}


void WvDBusServer::proxy_msg(uint32_t serial, WvDBusMsg &msg)
{
    log("Proxying %s -> #%s\n", msg, serial);
    
    WvDBusReplySerial *s = rsdict[serial];
    if (s)
    {
        log("Proxy reply: target is %s\n", s->conn->name);
        s->conn->send(msg);
        rsdict.remove(s);
    }
    else
        log(WvLog::Error, "Proxy reply: no connection for serial #%s!\n",
	    serial);
}

WvString WvDBusServer::get_addr()
{
    return priv->get_addr();
}
