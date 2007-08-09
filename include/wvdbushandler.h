/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2004-2006 Net Integration Technologies, Inc.
 * 
 * Pathfinder Software:
 *   Copyright (C) 2007, Carillon Information Security Inc.
 *
 * This library is licensed under the LGPL, please read LICENSE for details.
 */ 
#ifndef __WVDBUSHANDLER_H
#define __WVDBUSHANDLER_H

#include "iwvdbuslistener.h"
#include "wvhashtable.h"
#include "wvlog.h"

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
	assert(listener);
        if (!d[path])
            d.add(new WvDBusObject(path), true);

	assert(!d[path]->d[listener->member]);
        d[path]->d.add(listener, true);
    }

    void del_listener(WvStringParm path, WvStringParm name)
    {
        WvDBusObject *o = d[path];
	assert(o);

        IWvDBusListener *m = o->d[name];
	assert(m);

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
	    else
		log(WvLog::Warning, "No handler for '%s' in '%s'\n",
		    member, objname);
        }
    }

    WvString name; // FIXME: ideally wouldn't be public
    WvDBusObjectDict d;
    WvLog log;
};

DeclareWvDict(WvDBusInterface, WvString, name);


#endif // __WVDBUSHANDLER_H
