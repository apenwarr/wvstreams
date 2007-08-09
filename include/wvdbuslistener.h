/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2005-2006 Net Integration Technologies, Inc.
 * 
 * Pathfinder Software:
 *   Copyright (C) 2007, Carillon Information Security Inc.
 *
 * This library is licensed under the LGPL, please read LICENSE for details.
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


template<typename P1 = E, typename P2 = E, typename P3 = E, typename P4 = E,
    typename P5 = E>
class WvDBusListener : public IWvDBusListener
{
public:
    WvDBusListener(WvDBusConn *_conn, WvStringParm _path, 
                   WvCallback<void, WvDBusConn&, WvDBusMsg&> _cb)
	: IWvDBusListener(_path)
    {}

    virtual void dispatch(const WvDBusMsg &_msg) = 0;
};


template<typename P1, typename P2, typename P3, typename P4>
class WvDBusListener<P1, P2, P3, P4, E> : public IWvDBusListener
{
public:
    WvDBusListener(WvDBusConn *_conn, WvStringParm _path, 
		   WvCallback<void, WvDBusConn&, WvDBusMsg&, P1, P2, P3, P4, WvError> _cb) :
        IWvDBusListener(_path)
    {
        cb = _cb;
        conn = _conn;
    }

    virtual void dispatch(const WvDBusMsg &_msg)
    {
        WvError err;
	WvDBusMsg msg(_msg);
	WvDBusMsg::Iter i(msg);
	
	if (!i.next()) err.set(EINVAL);
	P1 p1 = i;
	if (!i.next()) err.set(EINVAL);
	P2 p2 = i;
	if (!i.next()) err.set(EINVAL);
	P3 p3 = i;
	if (!i.next()) err.set(EINVAL);
	P4 p4 = i;
        cb(*conn, msg, p1, p2, p3, p4, err);
    }

    WvCallback<void, WvDBusConn&, WvDBusMsg&, P1, P2, P3, P4, WvError> cb;
    WvDBusConn *conn;
};


template<typename P1, typename P2, typename P3>
class WvDBusListener<P1, P2, P3, E, E> : public IWvDBusListener
{
public:
    WvDBusListener(WvDBusConn *_conn, WvStringParm _path, 
		   WvCallback<void, WvDBusConn&, WvDBusMsg&, P1, P2, P3, WvError> _cb) :
        IWvDBusListener(_path)
    {
        cb = _cb;
        conn = _conn;
    }

    virtual void dispatch(const WvDBusMsg &_msg)
    {
        WvError err;
	WvDBusMsg msg(_msg);
	WvDBusMsg::Iter i(msg);
	
	if (!i.next()) err.set(EINVAL);
	P1 p1 = i;
	if (!i.next()) err.set(EINVAL);
	P2 p2 = i;
	if (!i.next()) err.set(EINVAL);
	P3 p3 = i;
        cb(*conn, msg, p1, p2, p3, err);
    }

    WvCallback<void, WvDBusConn&, WvDBusMsg&, P1, P2, P3, WvError> cb;
    WvDBusConn *conn;
};


template<typename P1, typename P2>
class WvDBusListener<P1, P2, E, E, E> : public IWvDBusListener
{
public:
    WvDBusListener(WvDBusConn *_conn, WvStringParm _path, 
		   WvCallback<void, WvDBusConn&, WvDBusMsg&, P1, P2, WvError> _cb) :
        IWvDBusListener(_path)
    {
        cb = _cb;
        conn = _conn;
    }

    virtual void dispatch(const WvDBusMsg &_msg)
    {
        WvError err;
	WvDBusMsg msg(_msg);
	WvDBusMsg::Iter i(msg);
	
	if (!i.next()) err.set(EINVAL);
	P1 p1 = i;
	if (!i.next()) err.set(EINVAL);
	P2 p2 = i;
        cb(*conn, msg, p1, p2, err);
    }

    WvCallback<void, WvDBusConn&, WvDBusMsg&, P1, P2, WvError> cb;
    WvDBusConn *conn;
};


template<typename P1>
class WvDBusListener<P1, E, E, E, E> : public IWvDBusListener
{
public:
    WvDBusListener(WvDBusConn *_conn, WvStringParm _path, 
		   WvCallback<void, WvDBusConn&, WvDBusMsg&, P1, WvError> _cb) :
        IWvDBusListener(_path)
    {
        cb = _cb;
        conn = _conn;
    }

    virtual void dispatch(const WvDBusMsg &_msg)
    {
        WvError err;
	WvDBusMsg msg(_msg);
	WvDBusMsg::Iter i(msg);
	
	if (!i.next()) err.set(EINVAL);
	P1 p1 = i;
        cb(*conn, msg, p1, err);
    }

    WvCallback<void, WvDBusConn&, WvDBusMsg&, P1, WvError> cb;
    WvDBusConn *conn;
};


template <>
class WvDBusListener<E, E, E, E, E> : public IWvDBusListener
{
public:
    WvDBusListener(WvDBusConn *_conn, WvStringParm _path, 
                         WvCallback<void, WvDBusConn&, WvDBusMsg&, WvError> _cb) :
        IWvDBusListener(_path)
    {
        cb = _cb;
        conn = _conn;
    }

    virtual void dispatch(const WvDBusMsg &_msg)
    {
        WvError err;
	WvDBusMsg msg(_msg);
        cb(*conn, msg, err);
    }

    WvCallback<void, WvDBusConn&, WvDBusMsg&, WvError> cb;
    WvDBusConn *conn;
};


#endif // __WVDBUSLISTENER_H
