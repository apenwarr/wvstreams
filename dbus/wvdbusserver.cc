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
#include "wvdbusserver.h"
#include "wvdbuswatch.h"
#include "wvdbuslistener.h"
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


WvDBusServConn::WvDBusServConn(DBusConnection *_c, WvDBusServer *_s) :
    WvDBusConn(_c),
    server(_s)
{
    WvCallback<void, WvDBusConn &, WvDBusMsg &, WvError> cb1(
        this, &WvDBusServConn::hello_cb);
    WvDBusListener<> *l1 =
        new WvDBusListener<>(this, "Hello", cb1);

    WvCallback<void, WvDBusConn&, WvDBusMsg&, WvString, uint32_t, WvError> cb2(
        this, &WvDBusServConn::request_name_cb);
    WvDBusListener<WvString, uint32_t> *l2 =
        new WvDBusListener<WvString, uint32_t>(this, "RequestName", cb2);

    WvCallback<void, WvDBusConn&, WvDBusMsg&, WvString, WvError> cb3(
        this, &WvDBusServConn::release_name_cb);
    WvDBusListener<WvString> *l3 =
        new WvDBusListener<WvString>(this, "ReleaseName", cb3);

    add_method("org.freedesktop.DBus", "/org/freedesktop/DBus", l1);
    add_method("org.freedesktop.DBus", "/org/freedesktop/DBus", l2);
    add_method("org.freedesktop.DBus", "/org/freedesktop/DBus", l3);
}


void WvDBusServConn::hello_cb(WvDBusConn &conn, WvDBusMsg &msg, WvError err)
{    
    log("hello_cb\n");
    msg.reply().append(WvString(":%s", rand())).send(conn);
}


void WvDBusServConn::request_name_cb(WvDBusConn &conn, WvDBusMsg &msg, 
                                     WvString _name, uint32_t flags,
				     WvError err)
{
    log("request_name_cb(%s)\n", _name);
    if (!err.isok())
    {
        log("RequestName method called, but there was an error (%s).\n",
            err.errstr());
        return;
    }
    name = _name;
    server->register_conn(this);
    assert(server->cdict[_name] == this);

    msg.reply().append((uint32_t)DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER)
	.send(conn);
}


void WvDBusServConn::release_name_cb(WvDBusConn &conn, WvDBusMsg &msg, 
                                     WvString _name, WvError err)
{
    log("release_name_cb(%s)\n", _name);
    if (!err.isok())
    {
        log("ReleaseName method called, but there was an error (%s).\n",
            err.errstr());
        return;
    }
    name = "";

    msg.reply().append((uint32_t)DBUS_RELEASE_NAME_REPLY_RELEASED).send(conn);
}



bool WvDBusServConn::filter_func(WvDBusConn &conn, WvDBusMsg &msg)
{
    WvString path(msg.get_path());
    
    log("Filter: %s\n", msg);
    
    // the only object the server exports is "/org/freedesktop/DBus":
    // if that's not the destination, we need to proxy it to one of
    // our connected clients.
    if (!!path && path != "/org/freedesktop/DBus")
    {
	log("filter: %s serial=%s\n", msg,
	    dbus_message_get_reply_serial(msg));
	server->proxy_msg(msg.get_dest(), this, msg);
        return true;
    }
    else if (!path && msg.get_serial()) // most likely a reply
    {
	server->proxy_msg(msg.get_serial(), msg);
	return true;
    }
    return WvDBusConn::filter_func(conn, msg);
}

