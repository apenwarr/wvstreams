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

#include "wvdbushandler.h"
#include "wvistreamlist.h"
#include "wvlog.h"

class IWvDBusListener;
class WvDBusMsg;

struct DBusConnection;
struct DBusError;
struct DBusWatch;
struct DBusTimeout;
struct DBusPendingCall;


class WvDBusConn : public WvIStreamList
{
    friend class WvDBusConnHelpers;
public:
    enum BusType { BusSession = 0, BusSystem, BusStarter, NUM_BUS_TYPES };
    
    WvString name; // needs to be public for lookup
    WvLog log;

    /**
     * Creates a new dbus connection on a default bus (DBUS_BUS_SESSION or
     * DBUS_BUS_SYSTEM).
     */
    WvDBusConn(BusType bus = BusSession);
    
    /**
     * Creates a new dbus connection on a bus with the prescribed address.
     * Useful when you want to set up a connection to a custom server.
     */
    WvDBusConn(WvStringParm dbus_moniker);
    
    /**
     * Initialize this object from an existing low-level DBusConnection object.
     */
    WvDBusConn(DBusConnection *_c);
    
    /**
     * Release this connection.  If this is the last owner of the associated
     * DBusConnection object, the connection itself closes.
     */
    virtual ~WvDBusConn();

    /**
     * Request the given service name on DBus.  There's no guarantee the
     * server will let us have the requested name, though.
     * 
     * The name will be released when this connection object is destroyed.
     */
    void request_name(WvStringParm name);
    
    /**
     * Called by WvStreams when incoming data is ready.
     */
    virtual void execute();
    
    /**
     * Close the underlying stream.  The object becomes unusable.  This is
     * also called whenever an error is set.
     */
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

    /**
     * Set the error code of this connection if 'e' is nonempty.
     */
    void maybe_seterr(DBusError &e);

    /**
     * Called by DBus for each incoming message.  Returns true if we handled
     * this message, false if not.
     */
    virtual bool filter_func(WvDBusConn &conn, WvDBusMsg &msg);
    
private:
    void init(bool client);

    bool add_watch(DBusWatch *watch);
    void remove_watch(DBusWatch *watch);
    void watch_toggled(DBusWatch *watch);

    bool add_timeout(DBusTimeout *timeout);
    void remove_timeout(DBusTimeout *timeout);
    void timeout_toggled(DBusTimeout *timeout);

    static void pending_call_notify(DBusPendingCall *pending, void *user_data);
    static void remove_listener_cb(void *memory);

    WvDBusInterfaceDict ifacedict;
    DBusConnection *dbusconn;
    bool name_acquired;
    
    void _add_listener(WvStringParm interface, WvStringParm path,
		       IWvDBusListener *listener);
    void _del_listener(WvStringParm interface, WvStringParm path,
		       WvStringParm name);

};

#endif // __WVDBUSCONN_H
