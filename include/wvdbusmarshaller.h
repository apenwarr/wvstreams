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
#include <stdint.h>


bool convert_next(DBusMessageIter *iter, int &i);
bool convert_next(DBusMessageIter *iter, uint32_t &i);
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
        convert_next(&iter, p1); 
        cb(p1);
    }

    WvCallback<void, P1> cb;
};


template<typename P1 = E, typename P2 = E, typename P3 = E>
class WvDBusListener : public IWvDBusMarshaller
{
public:
    WvDBusListener(WvDBusConn *_conn, WvStringParm _path, 
                   WvCallback<void, WvDBusReplyMsg&> _cb) :
        IWvDBusMarshaller(_path)
    {}

    virtual void dispatch(DBusMessage *_msg)
    {}
};


template <>
class WvDBusListener<E, E, E>  : public IWvDBusMarshaller
{
public:
    WvDBusListener(WvDBusConn *_conn, WvStringParm _path, 
                   WvCallback<void, WvDBusReplyMsg&> _cb) :
        IWvDBusMarshaller(_path)
    {
        cb = _cb;
        conn = _conn;
    }

    virtual void dispatch(DBusMessage *_msg)
    {
        WvDBusReplyMsg msg(_msg);
        cb(msg);
        fprintf(stderr, "Dispatching reply..\n");
        conn->send(msg);
    }

    WvCallback<void, WvDBusReplyMsg&> cb;
    WvDBusConn *conn;
};


template<typename P1>
class WvDBusListener<P1, E, E> : public IWvDBusMarshaller
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
        DBusMessageIter iter;
        dbus_message_iter_init(_msg, &iter);
        //assert(dbus_message_iter_has_next(&iter));
        P1 p1;
        convert_next(&iter, p1);

        WvDBusReplyMsg msg(_msg);
        cb(msg, p1);
        fprintf(stderr, "Dispatching reply..\n");
        conn->send(msg);
        fprintf(stderr, "Reply sent..\n");
    }

    WvCallback<void, WvDBusReplyMsg&, P1> cb;
    WvDBusConn *conn;
};


template<typename P1, typename P2>
class WvDBusListener<P1, P2, E> : public IWvDBusMarshaller
{
public:
    WvDBusListener(WvDBusConn *_conn, WvStringParm _path, 
                   WvCallback<void, WvDBusReplyMsg&, P1, P2> _cb) :
        IWvDBusMarshaller(_path)
    {
        cb = _cb;
        conn = _conn;
    }

    virtual void dispatch(DBusMessage *_msg)
    {
        WvDBusReplyMsg msg(_msg);
        DBusMessageIter iter;
        dbus_message_iter_init(_msg, &iter);
        //assert(dbus_message_iter_has_next(&iter));
        P1 p1;
        bool error = convert_next(&iter, p1);
        P2 p2;
        error = convert_next(&iter, p2);
        cb(msg, p1, p2);
        fprintf(stderr, "Dispatching reply..\n");
        conn->send(msg);
    }

    WvCallback<void, WvDBusReplyMsg&, P1, P2> cb;
    WvDBusConn *conn;
};

#endif // __WVDBUSMARSHALLER_H
