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
#include "wvdbusservconn.h"
#include "wvdbuslistener.h"


class WvDBusServConnPrivate : public WvDBusConnPrivate
{
public:
    WvDBusServConnPrivate(WvDBusConn *_conn, DBusConnection *_c, 
                          WvDBusServer *_s) :
        WvDBusConnPrivate("DBus Serv Conn", _conn, _c),
        server(_s)
    {
    }

    virtual ~WvDBusServConnPrivate()
    {
    }
    
    virtual DBusHandlerResult filter_func(DBusConnection *_conn,
                                          DBusMessage *_msg)
    {
        WvString path = dbus_message_get_path(_msg);
        
        // the only object the server exports is "/org/freedesktop/DBus":
        // if that's not the destination, we need to proxy it to one of
        // our connected clients.
        if (!!path && path != "/org/freedesktop/DBus")
        {
            print_message_trace(_msg);
            log("filter: ->%s:%s serial=%s\n",
                dbus_message_get_destination(_msg), path,
                dbus_message_get_reply_serial(_msg));            
            WvDBusMsg msg(_msg);

            server->proxy_msg(dbus_message_get_destination(_msg), conn, msg);
        
            return DBUS_HANDLER_RESULT_HANDLED;            
        }
        else if (!path) // most likely a reply
        {
            WvDBusMsg msg(_msg);
            server->proxy_msg(dbus_message_get_reply_serial(_msg), msg);
            return DBUS_HANDLER_RESULT_HANDLED;
        }
        return WvDBusConnPrivate::filter_func(_conn, _msg);
    }

    WvDBusServer *server;
};


WvDBusServConn::WvDBusServConn(DBusConnection *_c, WvDBusServer *_s) :
    WvDBusConn(),
    server(_s),
    log("DBus Serv Conn")
{
    priv = new WvDBusServConnPrivate(this, _c, _s);

    WvCallback<void, WvDBusConn &, WvDBusReplyMsg &, WvError> cb1(
        this, &WvDBusServConn::hello_cb);
    WvDBusMethodListener<> *l1 =
        new WvDBusMethodListener<>(this, "Hello", cb1);

    WvCallback<void, WvDBusConn&, WvDBusReplyMsg&, WvString, uint32_t, WvError> cb2(
        this, &WvDBusServConn::request_name_cb);
    WvDBusMethodListener<WvString, uint32_t> *l2 =
        new WvDBusMethodListener<WvString, uint32_t>(this, "RequestName", cb2);

    WvCallback<void, WvDBusConn&, WvDBusReplyMsg&, WvString, WvError> cb3(
        this, &WvDBusServConn::release_name_cb);
    WvDBusMethodListener<WvString> *l3 =
        new WvDBusMethodListener<WvString>(this, "ReleaseName", cb3);

    add_method("org.freedesktop.DBus", "/org/freedesktop/DBus", l1);
    add_method("org.freedesktop.DBus", "/org/freedesktop/DBus", l2);
    add_method("org.freedesktop.DBus", "/org/freedesktop/DBus", l3);
}


void WvDBusServConn::hello_cb(WvDBusConn &conn, WvDBusReplyMsg &reply, WvError err)
{    
    reply.append(WvString(":%s", rand()));
    conn.send(reply);
}


void WvDBusServConn::request_name_cb(WvDBusConn &conn, WvDBusReplyMsg &reply, 
                                     WvString _name, uint32_t flags, WvError err)
{
    if (!err.isok())
    {
        log("RequestName method called, but there was an error (%s).\n",
            err.errstr());
        return;
    }
    reply.append((uint32_t)DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER);
    name = _name;
    server->register_conn(this);

    conn.send(reply);
}


void WvDBusServConn::release_name_cb(WvDBusConn &conn, WvDBusReplyMsg &reply, 
                                     WvString _name, WvError err)
{
    if (!err.isok())
    {
        log("ReleaseName method called, but there was an error (%s).\n",
            err.errstr());
        return;
    }

    reply.append((uint32_t)DBUS_RELEASE_NAME_REPLY_RELEASED);
    name = "";

    conn.send(reply);
}

