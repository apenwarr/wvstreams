/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2004-2006 Net Integration Technologies, Inc.
 * 
 * Pathfinder Software:
 *   Copyright (C) 2007, Carillon Information Security Inc.
 *
 * This library is licensed under the LGPL, please read LICENSE for details.
 *
 * A WvDBusConn represents a connection to another application. Messages
 * can be sent and received via this connection. In most cases, the
 * other application is a message bus. 
 */ 
#ifndef __WVDBUSCONN_H
#define __WVDBUSCONN_H

#include "wvistreamlist.h"
#include "wvlog.h"

class IWvDBusListener;
class WvDBusMsg;

struct DBusConnection;
struct DBusError;


class WvDBusConnBase : public WvIStreamList
{
public:
    enum BusType { BusSession = 0, BusSystem, BusStarter, NUM_BUS_TYPES };

#if 0
    /**
     * Creates a new dbus connection on a default bus (DBUS_BUS_SESSION or
     * DBUS_BUS_SYSTEM).
     */
    WvDBusConn(BusType bus);
    
    /**
     * Creates a new dbus connection on a bus with the prescribed address.
     * Useful when you want to set up a connection to a custom server.
     */
    WvDBusConnBase(WvStringParm dbus_moniker);
#endif
    WvDBusConnBase();

    virtual ~WvDBusConnBase();

    virtual void execute();
    virtual void close();
    
    /**
     * Send a message on the bus, returning the serial number that was
     * assigned to it.
     */
    virtual uint32_t send(WvDBusMsg &msg);
    
    /**
     * Send a message on the bus, calling reply() when the answer comes
     * back.
     */
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
    
    
    virtual DBusConnection *_getconn() const = 0;
    virtual void _add_listener(WvStringParm interface, WvStringParm path,
                              IWvDBusListener *listener) = 0;
    virtual void _del_listener(WvStringParm interface, WvStringParm path,
                              WvStringParm name) = 0;

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
    
    void maybe_seterr(DBusError &e);

    WvString name; // needs to be public for lookup
    WvLog log;
};

#include "wvdbusconnp.h" // FIXME temporary

#endif // __WVDBUSCONN_H
