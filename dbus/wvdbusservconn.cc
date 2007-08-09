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



DBusHandlerResult WvDBusServConn::filter_func(DBusConnection *_conn,
					      DBusMessage *_msg)
{
    WvDBusMsg msg(_msg);
    WvString path(msg.get_path());
    
    // the only object the server exports is "/org/freedesktop/DBus":
    // if that's not the destination, we need to proxy it to one of
    // our connected clients.
    if (!!path && path != "/org/freedesktop/DBus")
    {
	print_message_trace(msg);
	log("filter: %s serial=%s\n", msg,
	    dbus_message_get_reply_serial(msg));
	server->proxy_msg(msg.get_dest(), this, msg);
        
	return DBUS_HANDLER_RESULT_HANDLED;            
    }
    else if (!path) // most likely a reply
    {
	server->proxy_msg(dbus_message_get_reply_serial(_msg), msg);
	return DBUS_HANDLER_RESULT_HANDLED;
    }
    return WvDBusConn::filter_func(_conn, _msg);
}

