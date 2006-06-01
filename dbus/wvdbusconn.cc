/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2004-2006 Net Integration Technologies, Inc.
 * 
 */ 
#include "wvdbusconn.h"
#include "wvdbusconnp.h"
#include "wvdbuswatch.h"


WvDBusConn::WvDBusConn(WvStringParm _name, DBusBusType bus)
    : name(_name), 
      log("WvDBusConn")
{
    log("Starting up..\n");
    priv = new WvDBusConnPrivate(this, name, bus);

}


WvDBusConn::WvDBusConn(WvStringParm _name, WvStringParm address)
    : name(_name), 
      log("WvDBusConn")
{
    log("Starting up..\n");
    priv = new WvDBusConnPrivate(this, name, address);

}


WvDBusConn::WvDBusConn(DBusConnection *_c)
    : log("WvDBusConn")
{
    log("Starting up..");
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
    if (!dbus_connection_send(priv->dbusconn, msg, &serial)) 
    { 
        log(WvLog::Error, "Out Of Memory!\n"); 
        // FIXME: what do we do NOW?
    }
    else
        log(WvLog::Debug, "DBus message sent with serial %s\n", serial);
}


void WvDBusConn::send(WvDBusMsg &msg, IWvDBusMarshaller *reply, 
                      bool autofree_reply)
{
    log(WvLog::Debug, "Sending message.\n");
    DBusPendingCall * pending;

    // FIXME: allow custom timeouts?
    if (!dbus_connection_send_with_reply(priv->dbusconn, msg, &pending, 1000)) 
    { 
        log(WvLog::Error, "Out Of Memory!\n"); 
        // FIXME: what do we do NOW?
        return;
    }

    if (pending == NULL) 
    { 
        log(WvLog::Error, "Pending Call Null\n"); 
        // FIXME: what do we do NOW?
    }

    DBusFreeFunction free_user_data = NULL;
    if (autofree_reply)
        free_user_data = &WvDBusConnPrivate::remove_marshaller_cb;

    if (!dbus_pending_call_set_notify(pending, 
                                      &WvDBusConnPrivate::pending_call_notify,
                                      reply, free_user_data))
    {
        log(WvLog::Error, "Setting NOTIFY failed..\n"); 
        // FIXME: what do we do NOW?
    }

}


void WvDBusConn::add_listener(WvStringParm interface, WvStringParm path, 
                              IWvDBusMarshaller *marshaller)
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

    priv->add_marshaller(interface, path, marshaller);    
}


void WvDBusConn::del_listener(WvStringParm interface, WvStringParm path,
                              WvStringParm name)
{
    DBusError error;
    dbus_error_init(&error);

    dbus_bus_remove_match(priv->dbusconn, 
                          WvString("type='signal',interface='%s'", interface),  &error);
    if (dbus_error_is_set(&error))
    {
        log(WvLog::Error, "Oh no! Couldn't remove a match on the bus!\n");
        return;
    }

    priv->del_marshaller(interface, path, name);
}


void WvDBusConn::add_method(WvStringParm interface, WvStringParm path, 
                            IWvDBusMarshaller *listener)
{
    priv->add_marshaller(interface, path, listener);
}


void WvDBusConn::del_method(WvStringParm interface, WvStringParm path,
                            WvStringParm name)
{
    priv->del_marshaller(interface, path, name);
}
