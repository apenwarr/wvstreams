/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2004-2006 Net Integration Technologies, Inc.
 * 
 */ 
#ifndef __WVDBUSCONNP_H
#define __WVDBUSCONNP_H
#include <dbus/dbus.h>
#include "iwvdbuslistener.h"
#include "wvhashtable.h"
#include "wvlog.h"
#include "wvstring.h"

class WvDBusConn;

DeclareWvDict(IWvDBusListener, WvString, member);

class WvDBusObject
{
public:
    WvDBusObject(WvStringParm _path) : d(10) { path = _path; }
    WvString path;
    IWvDBusListenerDict d;
};

DeclareWvDict(WvDBusObject, WvString, path);

class WvDBusInterface 
{
public:
    WvDBusInterface(WvStringParm _name) :
        d(10),
        log("DBus Interface", WvLog::Debug)
    {
        name = _name;
    }

    void add_listener(WvString path, IWvDBusListener *listener)
    {
        if (!d[path])
            d.add(new WvDBusObject(path), true);

        // FIXME: what about duplicates?
        d[path]->d.add(listener, true);
    }

    void del_listener(WvStringParm path, WvStringParm name)
    {
        WvDBusObject *o = d[path];
        if (!o)
        {
            log(WvLog::Warning, "Attempted to delete listener from object "
                "'%s', but object does not exist! (name: %s)\n", path, name);
            return;
        }

        IWvDBusListener *m = o->d[name];
        if (!m)
        {
            log(WvLog::Warning, "Attempted to delete listener from object "
                "'%s', but name does not exist! (name: %s)\n", path, name);
            return;
        }

        o->d.remove(m);
    }

    void handle_signal(WvStringParm objname, WvStringParm member, 
                       WvDBusConn *conn, DBusMessage *msg)
    {
        if (d[objname])
        {
            WvDBusObject *obj = d[objname];
            if (obj->d[member])
                obj->d[member]->dispatch(msg);
        }
    }

    WvString name; // FIXME: ideally wouldn't be public
    WvDBusObjectDict d;
    WvLog log;
};

DeclareWvDict(WvDBusInterface, WvString, name);


class WvDBusConnPrivate
{
public:
    WvDBusConnPrivate(WvDBusConn *_conn, WvStringParm _name, DBusBusType bus);
    WvDBusConnPrivate(WvDBusConn *_conn, WvStringParm _name, 
                      WvStringParm _address);
    WvDBusConnPrivate(WvDBusConn *_conn, DBusConnection *_c);
    virtual ~WvDBusConnPrivate();
    
    void init(bool client);
    void request_name(WvStringParm name);

    void add_listener(WvStringParm interface, WvStringParm path, 
                        IWvDBusListener *listener);
    void del_listener(WvStringParm interface,  WvStringParm path, 
                        WvStringParm name);

    virtual dbus_bool_t add_watch(DBusWatch *watch);
    virtual void remove_watch(DBusWatch *watch);
    virtual void watch_toggled(DBusWatch *watch);

    virtual dbus_bool_t add_timeout(DBusTimeout *timeout);
    virtual void remove_timeout(DBusTimeout *timeout);
    virtual void timeout_toggled(DBusTimeout *timeout);

    virtual DBusHandlerResult filter_func(DBusConnection *_conn,
                                          DBusMessage *_msg);
    void print_message_trace(DBusMessage *_msg);


    static void pending_call_notify(DBusPendingCall *pending, void *user_data);
    static void remove_listener_cb(void *memory);

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
