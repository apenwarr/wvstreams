/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2004-2006 Net Integration Technologies, Inc.
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
        WvDBusConnPrivate(_conn, _c),
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
            log("Here we are! (path: %s, dest: %s reply serial: %s)\n", path,
                dbus_message_get_destination(_msg),
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
    WvDBusConn(_c),
    server(_s),
    log("WvDBusServConn")
{
    priv = new WvDBusServConnPrivate(this, _c, _s);

    WvCallback<void, WvDBusReplyMsg &> cb1(
        this, &WvDBusServConn::hello_cb);
    WvDBusMethodListener<> *l1 =
        new WvDBusMethodListener<>(this, "Hello", cb1);

    WvCallback<void, WvDBusReplyMsg &, WvString, uint32_t> cb2(
        this, &WvDBusServConn::request_name_cb);
    WvDBusMethodListener<WvString, uint32_t> *l2 =
        new WvDBusMethodListener<WvString, uint32_t>(this, "RequestName", cb2);

    WvCallback<void, WvDBusReplyMsg &, WvString> cb3(
        this, &WvDBusServConn::release_name_cb);
    WvDBusMethodListener<WvString> *l3 =
        new WvDBusMethodListener<WvString>(this, "ReleaseName", cb3);

    add_listener("org.freedesktop.DBus", "/org/freedesktop/DBus", l1);
    add_listener("org.freedesktop.DBus", "/org/freedesktop/DBus", l2);
    add_listener("org.freedesktop.DBus", "/org/freedesktop/DBus", l3);
}


void WvDBusServConn::hello_cb(WvDBusReplyMsg &reply)
{
    // whee!
    reply.append(WvString(":%s", rand()));
}


void WvDBusServConn::request_name_cb(WvDBusReplyMsg &reply, WvString _name, 
                                     uint32_t flags)
{
    // whee!
    reply.append(DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER);
    name = _name;
    server->register_conn(this);
}


void WvDBusServConn::release_name_cb(WvDBusReplyMsg &reply, WvString _name)
{
    // whee!
    reply.append(DBUS_RELEASE_NAME_REPLY_RELEASED);
    name = "";
}


void WvDBusServConn::add_listener(WvStringParm interface, WvStringParm path, 
                                    IWvDBusListener *listener)
{
    priv->add_listener(interface, path, listener);
}
