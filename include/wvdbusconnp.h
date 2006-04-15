/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2004-2006 Net Integration Technologies, Inc.
 * 
 */ 
#ifndef __WVDBUSCONNP_H
#define __WVDBUSCONNP_H
#include <dbus/dbus.h>
#include "iwvdbusmarshaller.h"
#include "wvhashtable.h"
#include "wvlog.h"
#include "wvstring.h"

class WvDBusConn;

DeclareWvDict(IWvDBusMarshaller, WvString, member);

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

DeclareWvDict(WvDBusInterface, WvString, name);


class WvDBusConnPrivate
{
public:
    WvDBusConnPrivate(WvDBusConn *_conn, WvStringParm _name, DBusBusType bus);
    WvDBusConnPrivate(WvDBusConn *_conn, WvStringParm _name, 
                      WvStringParm _address);
    WvDBusConnPrivate(WvDBusConn *_conn, DBusConnection *_c);
    ~WvDBusConnPrivate();

    void init(bool client);
    void request_name(WvStringParm name);

    void add_marshaller(WvStringParm interface, WvStringParm path, 
                        IWvDBusMarshaller *marshaller);

    static dbus_bool_t add_watch(DBusWatch *watch, void *data);
    static void remove_watch(DBusWatch *watch, void *data);
    static void watch_toggled(DBusWatch *watch, void *data);

    static dbus_bool_t add_timeout(DBusTimeout *timeout, void *data);
    static void remove_timeout(DBusTimeout *timeout, void *data);
    static void timeout_toggled(DBusTimeout *timeout, void *data);

    static DBusHandlerResult filter_func(DBusConnection *_conn,
                                         DBusMessage *_msg,
                                         void *userdata);

    static void pending_call_notify(DBusPendingCall *pending, void *user_data);
    static void remove_marshaller_cb(void *memory);

    void execute();
    void close();

    WvDBusConn *conn;
    WvDBusInterfaceDict ifacedict;
    DBusConnection *dbusconn;
    WvString name;
    bool name_acquired;

    WvLog log;
};

#endif // __WVDBUSCONNP_H
