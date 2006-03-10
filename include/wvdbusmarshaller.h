/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2005 Net Integration Technologies, Inc.
 * 
 */ 
#ifndef __WVDBUSMARSHALLER_H
#define __WVDBUSMARSHALLER_H
#include "iwvdbusmarshaller.h"
#include "wvcallback.h"
#include "wvdbusconn.h"
#include "wvstring.h"

#include <assert.h>
#include <dbus/dbus.h>

#if 0
template<typename P1 = E,
	 typename P2 = E,
	 typename P3 = E,
	 typename P4 = E,
	 typename P5 = E,
	 typename P6 = E,
	 typename P7 = E,
	 typename P8 = E>
class WvDBusMarshaller : public IWvDBusMarshaller
{
public:
    WvDBusMarshaller(WvStringParm _path, 
                     WvCallback<void, P1, P2, P3, P4, P5, P6, P7, P8> _cb) :
        IWvDBusMarshaller(_path)
    {
        cb = _cb;
    }
    virtual void dispatch(DBusMessage *_msg) = 0;

    WvCallback<void, P1, P2, P3, P4, P5, P6, P7, P8> cb;
};
#endif


bool convert_next(DBusMessageIter *iter, int &i);
bool convert_next(DBusMessageIter *iter, WvString &s);


template<typename P1>
class WvDBusMarshaller : public IWvDBusMarshaller
{
public:
    WvDBusMarshaller(WvStringParm _path, WvCallback<void, P1> _cb) :
        IWvDBusMarshaller(_path)
    {
        cb = _cb;
    }

    virtual void dispatch(DBusMessage *_msg)
    {
        DBusMessageIter iter;
        dbus_message_iter_init(_msg, &iter);
        //assert(dbus_message_iter_has_next(&iter));
        P1 p1;
        bool error = convert_next(&iter, p1); 
        cb(p1);
    }

    WvCallback<void, P1> cb;
};


template<typename P1>
class WvDBusListener : public IWvDBusMarshaller
{
public:
    WvDBusListener(WvDBusConn *_conn, WvStringParm _path, 
                   WvCallback<void, WvDBusReplyMsg&, P1> _cb) :
        IWvDBusMarshaller(_path)
    {
        cb = _cb;
        conn = _conn;
    }

    virtual void dispatch(DBusMessage *_msg)
    {
        fprintf(stderr, "Hello?\n");

        WvDBusReplyMsg msg(_msg);
        DBusMessageIter iter;
        dbus_message_iter_init(_msg, &iter);
        //assert(dbus_message_iter_has_next(&iter));
        P1 p1;
        bool error = convert_next(&iter, p1);
        cb(msg, p1);
        fprintf(stderr, "Dispatching reply..\n");
        conn->send(msg);
    }

    WvCallback<void, WvDBusReplyMsg&, P1> cb;
    WvDBusConn *conn;
};

#endif // __WVDBUSMARSHALLER_H
