/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2005-2006 Net Integration Technologies, Inc.
 * 
 */ 
#ifndef __WVDBUSLISTENER_H
#define __WVDBUSLISTENER_H
#include "iwvdbuslistener.h"
#include "wvcallback.h"
#include "wvdbusconn.h"
#include "wvstring.h"

#include <assert.h>
#include <dbus/dbus.h>
#include <stdint.h>


void convert_next(DBusMessageIter *iter, bool &b, WvError &err);
void convert_next(DBusMessageIter *iter, char &c, WvError &err);
void convert_next(DBusMessageIter *iter, int16_t &i, WvError &err);
void convert_next(DBusMessageIter *iter, int32_t &i, WvError &err);
void convert_next(DBusMessageIter *iter, uint16_t &i, WvError &err);
void convert_next(DBusMessageIter *iter, uint32_t &i, WvError &err);
void convert_next(DBusMessageIter *iter, double &d, WvError &err);
void convert_next(DBusMessageIter *iter, WvString &s, WvError &err);


template<typename P1>
class WvDBusListener : public IWvDBusListener
{
public:
    WvDBusListener(WvStringParm _path, 
                         WvCallback<void, P1, WvError> _cb) :
        IWvDBusListener(_path)
    {
        cb = _cb;
    }

    virtual void dispatch(DBusMessage *_msg)
    {
        WvError err;
        DBusMessageIter iter;
        dbus_message_iter_init(_msg, &iter);
        P1 p1;
        convert_next(&iter, p1, err);
        cb(p1, err);
    }

    WvCallback<void, P1, WvError> cb;
};


template<typename P1 = E, typename P2 = E, typename P3 = E>
class WvDBusMethodListener : public IWvDBusListener
{
public:
    WvDBusMethodListener(WvDBusConn *_conn, WvStringParm _path, 
                   WvCallback<void, WvDBusReplyMsg&> _cb) :
        IWvDBusListener(_path)
    {}

    virtual void dispatch(DBusMessage *_msg)
    {}
};


template <>
class WvDBusMethodListener<E, E, E>  : public IWvDBusListener
{
public:
    WvDBusMethodListener(WvDBusConn *_conn, WvStringParm _path, 
                         WvCallback<void, WvDBusReplyMsg&, WvError> _cb) :
        IWvDBusListener(_path)
    {
        cb = _cb;
        conn = _conn;
    }

    virtual void dispatch(DBusMessage *_msg)
    {
        WvError err;

        WvDBusReplyMsg msg(_msg);
        cb(msg, err);

        conn->send(msg);
    }

    WvCallback<void, WvDBusReplyMsg&, WvError> cb;
    WvDBusConn *conn;
};


template<typename P1>
class WvDBusMethodListener<P1, E, E> : public IWvDBusListener
{
public:
    WvDBusMethodListener(WvDBusConn *_conn, WvStringParm _path, 
                         WvCallback<void, WvDBusReplyMsg&, P1, WvError> _cb) :
        IWvDBusListener(_path)
    {
        cb = _cb;
        conn = _conn;
    }

    virtual void dispatch(DBusMessage *_msg)
    {
        WvError err;

        DBusMessageIter iter;
        dbus_message_iter_init(_msg, &iter);
        P1 p1;
        convert_next(&iter, p1, err);

        WvDBusReplyMsg msg(_msg);
        cb(msg, p1, err);
        if (err.isok())
            conn->send(msg);
    }

    WvCallback<void, WvDBusReplyMsg&, P1, WvError> cb;
    WvDBusConn *conn;
};


template<typename P1, typename P2>
class WvDBusMethodListener<P1, P2, E> : public IWvDBusListener
{
public:
    WvDBusMethodListener(WvDBusConn *_conn, WvStringParm _path, 
                         WvCallback<void, WvDBusReplyMsg&, P1, P2, WvError> _cb) :
        IWvDBusListener(_path)
    {
        cb = _cb;
        conn = _conn;
    }

    virtual void dispatch(DBusMessage *_msg)
    {
        WvError err;

        WvDBusReplyMsg msg(_msg);
        DBusMessageIter iter;
        dbus_message_iter_init(_msg, &iter);

        P1 p1;
        convert_next(&iter, p1, err);
        P2 p2;
        convert_next(&iter, p2, err);

        cb(msg, p1, p2, err);
        if (err.isok())
            conn->send(msg);
    }

    WvCallback<void, WvDBusReplyMsg&, P1, P2, WvError> cb;
    WvDBusConn *conn;
};

#endif // __WVDBUSLISTENER_H
