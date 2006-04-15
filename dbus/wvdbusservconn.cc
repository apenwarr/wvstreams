/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2004-2006 Net Integration Technologies, Inc.
 * 
 */ 
#include "wvdbusconnp.h"
#include "wvdbusservconn.h"
#include "wvdbusmarshaller.h"


WvDBusServConn::WvDBusServConn(DBusConnection *_c) :
    WvDBusConn(_c),
    log("WvDBusServConn")
{

    WvCallback<void, WvDBusReplyMsg &> cb1(
        this, &WvDBusServConn::hello_cb);
    WvDBusListener<> *l1 =
        new WvDBusListener<>(this, "Hello", cb1);

    WvCallback<void, WvDBusReplyMsg &, WvString, uint32_t> cb2(
        this, &WvDBusServConn::request_name_cb);
    WvDBusListener<WvString, uint32_t> *l2 =
        new WvDBusListener<WvString, uint32_t>(this, "RequestName", cb2);

    add_marshaller("org.freedesktop.DBus", "/org/freedesktop/DBus", l1);
    add_marshaller("org.freedesktop.DBus", "/org/freedesktop/DBus", l2);
}


void WvDBusServConn::hello_cb(WvDBusReplyMsg &reply)
{
    // whee!
    reply.append(WvString("%s", rand() % 10000));
}


void WvDBusServConn::request_name_cb(WvDBusReplyMsg &reply, WvString name, 
                                     uint32_t flags)
{
    // whee!
    reply.append(DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER);
}


void WvDBusServConn::add_marshaller(WvStringParm interface, WvStringParm path, 
                                    IWvDBusMarshaller *marshaller)
{
    priv->add_marshaller(interface, path, marshaller);
}
