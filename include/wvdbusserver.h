/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2004-2006 Net Integration Technologies, Inc.
 * 
 */ 
#ifndef __WVDBUSSERVER_H
#define __WVDBUSSERVER_H
#include "iwvdbusmarshaller.h"
#include "wvdbusmsg.h"
#include "wvfdstream.h"
#include "wvhashtable.h"
#include "wvistreamlist.h"
#include "wvlog.h"
#include "wvstringlist.h"
#include <dbus/dbus.h>


class WvDBusServerPrivate;

class WvDBusServer : public WvIStreamList
{
public:
    /* 
     * Constructs a new DBus server at the specified address. 
     *
     * For example:
     *    WvDBusServer s("unix:path=/tmp/foo");
     *
     * @param address moniker to construct the server
     *
     */
    WvDBusServer(WvStringParm addr);
    WvDBusServer(WvDBusServer &c);
    virtual ~WvDBusServer();

    virtual bool isok() const
    {
        return true; 
    }

    virtual void execute();

private:
    WvDBusServerPrivate *priv;
    WvLog log;
};

#endif // __WVDBUSSERVER_H
