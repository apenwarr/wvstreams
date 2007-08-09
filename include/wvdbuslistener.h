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
#include <stdint.h>

void convert_next(DBusMessageIter *iter, bool &b, WvError &err);
void convert_next(DBusMessageIter *iter, char &c, WvError &err);
void convert_next(DBusMessageIter *iter, int16_t &i, WvError &err);
void convert_next(DBusMessageIter *iter, int32_t &i, WvError &err);
void convert_next(DBusMessageIter *iter, uint16_t &i, WvError &err);
void convert_next(DBusMessageIter *iter, uint32_t &i, WvError &err);
void convert_next(DBusMessageIter *iter, double &d, WvError &err);
void convert_next(DBusMessageIter *iter, WvString &s, WvError &err);


template<typename P1 = E, typename P2 = E, typename P3 = E, typename P4 = E>
class WvDBusListener : public IWvDBusListener
{
public:
    WvDBusListener(WvDBusConn *_conn, WvStringParm _path, 
                         WvCallback<void> _cb) :
        IWvDBusListener(_path)
    {}

    virtual void dispatch(const WvDBusMsg &_msg)
    {}
};


template<>
class WvDBusListener<> : public IWvDBusListener
{
public:
    WvDBusListener(WvStringParm _path, 
                         WvCallback<void, WvError> _cb) :
        IWvDBusListener(_path)
    {
        cb = _cb;
    }

    virtual void dispatch(const WvDBusMsg &_msg)
    {
        WvError err;
        cb(err);
    }

    WvCallback<void, WvError> cb;
};


template<typename P1>
class WvDBusListener<P1> : public IWvDBusListener
{
public:
    WvDBusListener(WvStringParm _path, 
                         WvCallback<void, P1, WvError> _cb) :
        IWvDBusListener(_path)
    {
        cb = _cb;
    }

    virtual void dispatch(const WvDBusMsg &_msg)
    {
        WvError err;
	WvDBusMsg::Iter i(_msg);
	
	if (!i.next()) err.set(EINVAL);
	P1 p1 = i;
        cb(p1, err);
    }

    WvCallback<void, P1, WvError> cb;
};


template<typename P1, typename P2>
class WvDBusListener<P1, P2> : public IWvDBusListener
{
public:
    WvDBusListener(WvStringParm _path, 
                   WvCallback<void, P1, P2, WvError> _cb) :
        IWvDBusListener(_path)
    {
        cb = _cb;
    }

    virtual void dispatch(const WvDBusMsg &_msg)
    {
        WvError err;
	WvDBusMsg::Iter i(_msg);
	
	if (!i.next()) err.set(EINVAL);
	P1 p1 = i;
	if (!i.next()) err.set(EINVAL);
	P2 p2 = i;
        cb(p1, p2, err);
    }

    WvCallback<void, P1, P2, WvError> cb;
};


template<typename P1, typename P2, typename P3>
class WvDBusListener<P1, P2, P3> : public IWvDBusListener
{
public:
    WvDBusListener(WvStringParm _path, 
                   WvCallback<void, P1, P2, P3, WvError> _cb) :
        IWvDBusListener(_path)
    {
        cb = _cb;
    }

    virtual void dispatch(const WvDBusMsg &_msg)
    {
        WvError err;
	WvDBusMsg::Iter i(_msg);
	
	if (!i.next()) err.set(EINVAL);
	P1 p1 = i;
	if (!i.next()) err.set(EINVAL);
	P2 p2 = i;
	if (!i.next()) err.set(EINVAL);
	P3 p3 = i;
        cb(p1, p2, p3, err);
    }

    WvCallback<void, P1, P2, P3, WvError> cb;
};


template<typename P1 = E, typename P2 = E, typename P3 = E, typename P4 = E>
class WvDBusMethodListener : public IWvDBusListener
{
public:
    WvDBusMethodListener(WvDBusConn *_conn, WvStringParm _path, 
                   WvCallback<void, WvDBusReplyMsg&> _cb) :
        IWvDBusListener(_path)
    {}

    virtual void dispatch(const WvDBusMsg &_msg)
    {}
};


template <>
class WvDBusMethodListener<>  : public IWvDBusListener
{
public:
    WvDBusMethodListener(WvDBusConn *_conn, WvStringParm _path, 
                         WvCallback<void, WvDBusReplyMsg&, WvError> _cb) :
        IWvDBusListener(_path)
    {
        cb = _cb;
        conn = _conn;
    }

    virtual void dispatch(const WvDBusMsg &_msg)
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
class WvDBusMethodListener<P1, E, E, E> : public IWvDBusListener
{
public:
    WvDBusMethodListener(WvDBusConn *_conn, WvStringParm _path, 
                         WvCallback<void, WvDBusReplyMsg&, P1, WvError> _cb) :
        IWvDBusListener(_path)
    {
        cb = _cb;
        conn = _conn;
    }

    virtual void dispatch(const WvDBusMsg &_msg)
    {
        WvError err;
	WvDBusMsg::Iter i(_msg);
	
	if (!i.next()) err.set(EINVAL);
	P1 p1 = i;
        WvDBusReplyMsg msg(_msg);
        cb(msg, p1, err);
        if (err.isok())
            conn->send(msg);
    }

    WvCallback<void, WvDBusReplyMsg&, P1, WvError> cb;
    WvDBusConn *conn;
};


template<typename P1, typename P2>
class WvDBusMethodListener<P1, P2, E, E> : public IWvDBusListener
{
public:
    WvDBusMethodListener(WvDBusConn *_conn, WvStringParm _path, 
                         WvCallback<void, WvDBusReplyMsg&, P1, P2, WvError> _cb) :
        IWvDBusListener(_path)
    {
        cb = _cb;
        conn = _conn;
    }

    virtual void dispatch(const WvDBusMsg &_msg)
    {
        WvError err;
	WvDBusMsg::Iter i(_msg);
	
	if (!i.next()) err.set(EINVAL);
	P1 p1 = i;
	if (!i.next()) err.set(EINVAL);
	P2 p2 = i;
        WvDBusReplyMsg msg(_msg);
        cb(msg, p1, p2, err);
        if (err.isok())
            conn->send(msg);
    }

    WvCallback<void, WvDBusReplyMsg&, P1, P2, WvError> cb;
    WvDBusConn *conn;
};


template<typename P1, typename P2, typename P3>
class WvDBusMethodListener<P1, P2, P3, E> : public IWvDBusListener
{
public:
    WvDBusMethodListener(WvDBusConn *_conn, WvStringParm _path, 
                         WvCallback<void, WvDBusReplyMsg&, P1, P2, P3, WvError> _cb) :
        IWvDBusListener(_path)
    {
        cb = _cb;
        conn = _conn;
    }

    virtual void dispatch(const WvDBusMsg &_msg)
    {
        WvError err;
	WvDBusMsg::Iter i(_msg);
	
	if (!i.next()) err.set(EINVAL);
	P1 p1 = i;
	if (!i.next()) err.set(EINVAL);
	P2 p2 = i;
	if (!i.next()) err.set(EINVAL);
	P3 p3 = i;
        WvDBusReplyMsg msg(_msg);
        cb(msg, p1, p2, p3, err);
        if (err.isok())
            conn->send(msg);
    }

    WvCallback<void, WvDBusReplyMsg&, P1, P2, P3, WvError> cb;
    WvDBusConn *conn;
};

#endif // __WVDBUSLISTENER_H
