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
#include "wvdbusconn.h"
#include "wvdbusconnp.h"
#include "wvdbuswatch.h"

static int conncount;

WvDBusConnBase::WvDBusConnBase()
    : log(WvString("DBus Conn #%s", getpid()*1000 + ++conncount))
{
    log("Initializing.\n");
}


WvDBusConnBase::~WvDBusConnBase()
{
    log("Shutting down.\n");
    close();
}


void WvDBusConnBase::execute()
{
    WvIStreamList::execute();
}


void WvDBusConnBase::close()
{
}


uint32_t WvDBusConnBase::send(WvDBusMsg &msg)
{
    uint32_t serial;
    log(WvLog::Debug, "Sending %s\n", msg);
    if (!dbus_connection_send(_getconn(), msg, &serial)) 
        seterr_both(ENOMEM, "Out of memory.\n");
    return serial;
}


void WvDBusConnBase::send(WvDBusMsg &msg, IWvDBusListener *reply, 
                      bool autofree_reply)
{
    log(WvLog::Debug, "Sending_w_r %s\n", msg);
    DBusPendingCall *pending;

    if (!dbus_connection_send_with_reply(_getconn(), msg, &pending, -1))
    { 
        seterr_both(ENOMEM, "Out of memory.\n");
        return;
    }

    DBusFreeFunction free_user_data = NULL;
    if (autofree_reply)
        free_user_data = &WvDBusConn::remove_listener_cb;

    if (!dbus_pending_call_set_notify(pending, 
                                      &WvDBusConn::pending_call_notify,
                                      reply, free_user_data))
        seterr_both(ENOMEM, "Out of memory.\n");
}


void WvDBusConnBase::add_listener(WvStringParm interface, WvStringParm path, 
                              IWvDBusListener *listener)
{
    DBusError error;
    dbus_error_init(&error);

    dbus_bus_add_match(_getconn(), WvString("type='signal',interface='%s'",
                                                interface),  &error);
    maybe_seterr(error);
    if (isok())
	_add_listener(interface, path, listener);    
}


void WvDBusConnBase::del_listener(WvStringParm interface, WvStringParm path,
                              WvStringParm name)
{
    DBusError error;
    dbus_error_init(&error);

    dbus_bus_remove_match(_getconn(), 
                          WvString("type='signal',interface='%s'", interface),  
                          &error);
    maybe_seterr(error);
    if (isok())
	_del_listener(interface, path, name);
}


void WvDBusConnBase::add_method(WvStringParm interface, WvStringParm path, 
                            IWvDBusListener *listener)
{
    _add_listener(interface, path, listener);
}


void WvDBusConnBase::del_method(WvStringParm interface, WvStringParm path,
                            WvStringParm name)
{
    _del_listener(interface, path, name);
}


void WvDBusConnBase::maybe_seterr(DBusError &e)
{
    if (dbus_error_is_set(&e))
	seterr_both(EIO, "%s: %s", e.name, e.message);
}

