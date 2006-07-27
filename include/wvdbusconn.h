/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2004-2006 Net Integration Technologies, Inc.
 * 
 * A WvDBusConn represents a connection to another application. Messages
 * can be sent and received via this connection. In most cases, the
 * other application is a message bus. 
 */ 
#ifndef __WVDBUSCONN_H
#define __WVDBUSCONN_H
#include "iwvdbuslistener.h"
#include "wvdbusmsg.h"
#include "wvfdstream.h"
#include "wvhashtable.h"
#include "wvistreamlist.h"
#include "wvlog.h"
#include "wvstringlist.h"
#include <dbus/dbus.h>


class WvDBusConnPrivate;

class WvDBusConn : public WvIStreamList
{
public:
    /**
     * Creates a new dbus connection on a default bus (DBUS_BUS_SESSION or
     * DBUS_BUS_SYSTEM).
     */
    WvDBusConn(WvStringParm _name, DBusBusType bus = DBUS_BUS_SESSION);
    /**
     * Creates a new dbus connection on a bus with the prescribed address.
     * Useful when you want to set up a connection to a custom server.
     */
    WvDBusConn(WvStringParm _name, WvStringParm address);

    virtual ~WvDBusConn();

    virtual void execute();
    virtual void close();
    virtual void send(WvDBusMsg &msg);
    virtual void send(WvDBusMsg &msg, uint32_t &serial);
    virtual void send(WvDBusMsg &msg, IWvDBusListener *reply, 
                      bool autofree_reply);

    /**
     * Adds a signal listener to the bus connection: all signals matching 
     * the interface and path specification will be forwarded to the
     * appropriate listener.
     */
    virtual void add_listener(WvStringParm interface, WvStringParm path,
                              IWvDBusListener *listener);

    /**
     * Removes a signal listener from the bus connection.
     */
    virtual void del_listener(WvStringParm interface, WvStringParm path,
                              WvStringParm name);

    /**
     * Adds a method to the bus connection: all method calls matching
     * the interface and path specification will be forwarded to the 
     * appropriate listener. 
     */
    void add_method(WvStringParm interface, WvStringParm path, 
                    IWvDBusListener *listener);

    /**
     * Removes a method from the bus connection.
     */
    void del_method(WvStringParm interface, WvStringParm path,
                    WvStringParm name);

    operator DBusConnection* () const;

    WvString name; // needs to be public for lookup

protected:
    /**
     * Dummy constructor for WvDBusConn. Most useful for special applications, 
     * which want to set up the priv class themselves (e.g.: WvDBusServConn).
     */
    WvDBusConn();

    WvDBusConnPrivate *priv;
    WvLog log;
};

#endif // __WVDBUSCONN_H
