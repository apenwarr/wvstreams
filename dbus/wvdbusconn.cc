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


static DBusBusType bustypes[WvDBusConn::NUM_BUS_TYPES] 
    = { DBUS_BUS_SESSION, DBUS_BUS_SYSTEM, DBUS_BUS_STARTER };

WvDBusConn::WvDBusConn(WvStringParm _name, BusType bus)
    : name(_name), 
      log("DBus Conn")
{
    priv = new WvDBusConnPrivate(log.app, this, name, bustypes[bus]);
}


WvDBusConn::WvDBusConn(WvStringParm _name, WvStringParm address)
    : name(_name), 
      log("DBus Conn")
{
    priv = new WvDBusConnPrivate(log.app, this, name, address);
}


WvDBusConn::WvDBusConn()
    : log("DBus Conn")
{
}


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


void WvDBusConn::send(WvDBusMsg &msg)
{
    uint32_t serial;
    send(msg, serial);
}


void WvDBusConn::send(WvDBusMsg &msg, uint32_t &serial)
{
    log(WvLog::Debug, "Sending %s\n", msg);
    if (!dbus_connection_send(priv->dbusconn, msg, &serial)) 
        seterr_both(ENOMEM, "Out of memory.\n");
}


void WvDBusConn::send(WvDBusMsg &msg, IWvDBusListener *reply, 
                      bool autofree_reply)
{
    log(WvLog::Debug, "Sending_w_r %s\n", msg);
    DBusPendingCall *pending;

    // FIXME: allow custom timeouts?
    if (!dbus_connection_send_with_reply(priv->dbusconn, msg, &pending, 1000)) 
    { 
        seterr_both(ENOMEM, "Out of memory.\n");
        return;
    }

    DBusFreeFunction free_user_data = NULL;
    if (autofree_reply)
        free_user_data = &WvDBusConnPrivate::remove_listener_cb;

    if (!dbus_pending_call_set_notify(pending, 
                                      &WvDBusConnPrivate::pending_call_notify,
                                      reply, free_user_data))
        seterr_both(ENOMEM, "Out of memory.\n");
}


void WvDBusConn::add_listener(WvStringParm interface, WvStringParm path, 
                              IWvDBusListener *listener)
{
    DBusError error;
    dbus_error_init(&error);

    dbus_bus_add_match(priv->dbusconn, WvString("type='signal',interface='%s'",
                                                interface),  &error);
    if (dbus_error_is_set(&error))
    {
        log(WvLog::Error, "Oh no! Couldn't add a match on the bus!\n");
        return;
    }

    priv->add_listener(interface, path, listener);    
}


void WvDBusConn::del_listener(WvStringParm interface, WvStringParm path,
                              WvStringParm name)
{
    DBusError error;
    dbus_error_init(&error);

    dbus_bus_remove_match(priv->dbusconn, 
                          WvString("type='signal',interface='%s'", interface),  
                          &error);
    if (dbus_error_is_set(&error))
    {
        log(WvLog::Error, "Oh no! Couldn't remove a match on the bus!\n");
        return;
    }

    priv->del_listener(interface, path, name);
}


void WvDBusConn::add_method(WvStringParm interface, WvStringParm path, 
                            IWvDBusListener *listener)
{
    priv->add_listener(interface, path, listener);
}


void WvDBusConn::del_method(WvStringParm interface, WvStringParm path,
                            WvStringParm name)
{
    priv->del_listener(interface, path, name);
}
