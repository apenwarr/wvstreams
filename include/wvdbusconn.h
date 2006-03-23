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


DeclareWvDict(IWvDBusMarshaller, WvString, path);

class WvDBusConn;

class WvDBusInterface 
{
public:
    WvDBusInterface(WvStringParm _name) :
        d(10)
    {
        name = _name;
    }

    void add_marshaller(IWvDBusMarshaller *marshaller)
    {
        // FIXME: what about duplicates?
        d.add(marshaller, true);
    }

    void handle_signal(WvStringParm path, WvDBusConn *conn, DBusMessage *msg)
    {
        fprintf(stderr, "path is '%s'\n", path.cstr());
        if (d[path])
            d[path]->dispatch(msg);
    }

    WvString name; // FIXME: ideally wouldn't be public
    IWvDBusMarshallerDict d; // FIXME: ditto
};


class WvDBusConnPrivate;

class WvDBusConn : public WvIStreamList
{
public:
    WvDBusConn(WvStringParm name, DBusBusType bus = DBUS_BUS_SESSION);
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
    virtual void send(WvDBusMsg &msg, IWvDBusMarshaller *reply, bool autofree_reply);

    void add_marshaller(WvStringParm ifacename, IWvDBusMarshaller *marshaller);
    void add_method(WvStringParm ifacename, IWvDBusMarshaller *listener);

    operator DBusConnection* () const;

private:
    WvDBusConnPrivate *priv;
    WvLog log;
};

#endif // __WVDBUSCONN_H
