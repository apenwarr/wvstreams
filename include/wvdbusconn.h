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
    WvDBusConn(WvStringParm name, DBusBusType bus = DBUS_BUS_SESSION);
    WvDBusConn(WvStringParm name, WvStringParm address);
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
    virtual void send(WvDBusMsg &msg, IWvDBusMarshaller *reply, 
                      bool autofree_reply);

    void add_marshaller(WvStringParm interface, WvStringParm path, 
                        IWvDBusMarshaller *marshaller);
    void add_method(WvStringParm interface, WvStringParm path, 
                    IWvDBusMarshaller *listener);

    operator DBusConnection* () const;

protected:
    WvDBusConnPrivate *priv;
    WvLog log;
};

#endif // __WVDBUSCONN_H
