/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2005 Net Integration Technologies, Inc.
 * 
 */ 
#ifndef __WVDBUSCONN_H
#define __WVDBUSCONN_H
#include "wvfdstream.h"
#include "wvhashtable.h"
#include "wvistreamlist.h"
#include "wvlog.h"
#include "wvstringlist.h"
#include <dbus/dbus.h>

class WvDBusMsg
{
public:
    WvDBusMsg(DBusMessage *_msg)
    {
        msg = _msg;
    }
    WvDBusMsg(const WvDBusMsg &m)
    {
        dbus_message_ref(m);
    }
    ~WvDBusMsg()
    {
        if (msg) dbus_message_unref(msg);
    }

    operator DBusMessage* () const
    {
        return msg;
    }

    void append(WvStringParm s1, WvStringParm s2 = WvString::null,
                WvStringParm s3 = WvString::null);
    
    void decode(WvStringList &l) const;
    
    WvString arg(int n) const;

private:
    mutable WvStringList args;
    mutable DBusMessage *msg;
};


typedef WvCallback<void, const WvDBusMsg& > WvDBusSignalCallback;

class WvDBusSignalHandler
{
public:
    WvDBusSignalHandler(WvStringParm _name, WvDBusSignalCallback _cb)
    {
        name = _name;
        cb = _cb;
    }

    WvString name;
    WvDBusSignalCallback cb;
};

DeclareWvDict(WvDBusSignalHandler, WvString, name);

class WvDBusConn;

class WvDBusInterface 
{
public:
    WvDBusInterface(WvStringParm _path) :
        d(10)
    {
        path = _path;
    }

    void connect_to_signal(WvDBusConn *conn, WvStringParm name, 
                           WvDBusSignalCallback cb)
    {
        // FIXME: check for duplicates?
        d.add(new WvDBusSignalHandler(name, cb), true);
    }

    void handle_signal(WvStringParm name, WvDBusMsg &msg)
    {
        if (d[name])
            d[name]->cb(msg);
    }

    WvString path; // FIXME: ideally wouldn't be public
    WvDBusSignalHandlerDict d; // FIXME: ditto
};


// private!! hide me soon

class WvDBusConnPrivate;

DeclareWvDict(WvDBusInterface, WvString, path);

class WvDBusConn : public WvIStreamList
{
public:
    WvDBusConn(DBusBusType bus = DBUS_BUS_SESSION);
    WvDBusConn(DBusConnection *c);
    WvDBusConn(WvDBusConn &c);
    virtual ~WvDBusConn();

    virtual bool isok() const
    {
        return true; //conn;
    }

    virtual void execute();
    virtual void close();

    // void send(const WvDBusMsg &msg);

    operator DBusConnection* () const
    {
        return NULL;
        //return conn;
    }

    void add_interface_filter(WvDBusInterface *_iface);

    // this should go into WvDBusWatch, or some similar private class
#if 0
    static dbus_bool_t add_watch(DBusWatch *watch, void *data);
    static void remove_watch(DBusWatch *watch, void *data);
    WvDBusWatch * get_watch(int fd);
#endif
    // this also should be privatized/hidden somehow
    static DBusHandlerResult filter_func(DBusConnection *conn,
					 DBusMessage *msg,
					 void *userdata);

    WvDBusInterfaceDict ifacedict;

private:
    WvDBusConnPrivate *priv;

    WvLog log;
};

#endif // __WVDBUSCONN_H
