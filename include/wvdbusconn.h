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


DeclareWvDict(IWvDBusMarshaller, WvString, member);


class WvDBusConn;


class WvDBusObject
{
public:
    WvDBusObject(WvStringParm _path) : d(10) { path = _path; }
    WvString path;
    IWvDBusMarshallerDict d;
};


DeclareWvDict(WvDBusObject, WvString, path);


class WvDBusInterface 
{
public:
    WvDBusInterface(WvStringParm _name) :
        d(10)
    {
        name = _name;
    }

    void add_marshaller(WvString path, IWvDBusMarshaller *marshaller)
    {
        if (!d[path])
            d.add(new WvDBusObject(path), true);

        // FIXME: what about duplicates?
        d[path]->d.add(marshaller, true);
    }

    void handle_signal(WvStringParm objname, WvStringParm member, 
                       WvDBusConn *conn, DBusMessage *msg)
    {
        fprintf(stderr, "objname is '%s'\n", objname.cstr());
        if (d[objname])
        {
            WvDBusObject *obj = d[objname];
            if (obj->d[member])
                obj->d[member]->dispatch(msg);
        }
    }

    WvString name; // FIXME: ideally wouldn't be public
    WvDBusObjectDict d;
};


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
    virtual void send(WvDBusMsg &msg, IWvDBusMarshaller *reply, bool autofree_reply);

    void add_marshaller(WvStringParm interface, WvStringParm path, 
                        IWvDBusMarshaller *marshaller);
    void add_method(WvStringParm interface, WvStringParm path, 
                    IWvDBusMarshaller *listener);

    operator DBusConnection* () const;

private:
    WvDBusConnPrivate *priv;
    WvLog log;
};

#endif // __WVDBUSCONN_H
