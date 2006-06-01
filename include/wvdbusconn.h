/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2004-2006 Net Integration Technologies, Inc.
 * 
 */ 
#ifndef __WVDBUSCONN_H
#define __WVDBUSCONN_H
#include "iwvdbusmarshaller.h"
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
    WvDBusConn(WvStringParm _name, DBusBusType bus = DBUS_BUS_SESSION);
    WvDBusConn(WvStringParm _name, WvStringParm address);
    WvDBusConn(DBusConnection *c);
    WvDBusConn(WvDBusConn &c);
    virtual ~WvDBusConn();

    virtual bool isok() const
    {
        return true; //conn;
    }

    virtual void execute();
    virtual void close();
    virtual void send(WvDBusMsg &msg);
    virtual void send(WvDBusMsg &msg, uint32_t &serial);
    virtual void send(WvDBusMsg &msg, IWvDBusMarshaller *reply, 
                      bool autofree_reply);

    /**
     * Adds a signal listener to the bus connection: all signals matching 
     * the interface and path specification will be forwarded to the
     * appropriate marshaller.
     */
    void add_listener(WvStringParm interface, WvStringParm path, 
                      IWvDBusMarshaller *marshaller);

    /**
     * Removes a signal listener from the bus connection.
     */
    void del_listener(WvStringParm interface, WvStringParm path,
                      WvStringParm name);

    /**
     * Adds a method to the bus connection: all method calls matching
     * the interface and path specification will be forwarded to the 
     * appropriate marshaller. 
     */
    void add_method(WvStringParm interface, WvStringParm path, 
                    IWvDBusMarshaller *listener);

    /**
     * Removes a method from the bus connection.
     */
    void del_method(WvStringParm interface, WvStringParm path,
                    WvStringParm name);

    operator DBusConnection* () const;

    WvString name; // needs to be public for lookup

protected:
    WvDBusConnPrivate *priv;
    WvLog log;
};

#endif // __WVDBUSCONN_H
