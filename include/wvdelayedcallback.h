/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2003 Net Integration Technologies, Inc.
 *
 */
#ifndef __WVDELAYEDCALLBACK_H
#define __WVDELAYEDCALLBACK_H

#include "wvcallback.h"
#include "wvistreamlist.h"

/**
 * A WvCallback wrapper that delays until the next tick of the WvIStreamList
 * main loop.
 *
 * There are restrictions on the type of the wrapped callback though:
 *   1. The return type must be void
 *   2. All parameter types must be copy-constructible value types
 */
template<class InnerCallback>
class WvDelayedCallback
{
private:
    typedef typename InnerCallback::ReturnType R;
    typedef typename InnerCallback::Parm1 P1;
    typedef typename InnerCallback::Parm2 P2;
    typedef typename InnerCallback::Parm3 P3;
    typedef typename InnerCallback::Parm4 P4;
    typedef typename InnerCallback::Parm5 P5;
    typedef typename InnerCallback::Parm6 P6;
    typedef typename InnerCallback::Parm7 P7;
    typedef typename InnerCallback::Parm8 P8;
    typedef WvCallbackImpl<R, P1, P2, P3, P4, P5, P6, P7, P8> Impl;
    typedef typename Impl::FrozenParams FrozenParams;
    InnerCallback cb;
    FrozenParams *frozen;
    WvStream *stream;

    void thaw(WvStream &stream, void *userdata)
    {
        cb.thaw(*frozen);
        stream.close();
    }

public:
    template<typename PtrToObject, typename PtrToMember>
    WvDelayedCallback(PtrToObject obj, PtrToMember member)
        : cb(InnerCallback(obj, member)), frozen(0), stream(new WvStream)
    {
        stream->setcallback(WvStreamCallback(this, &WvDelayedCallback::thaw), 0);
        WvIStreamList::globallist.append(stream, true);
    }
    template<typename Functor>
    WvDelayedCallback(const Functor& func)
        : cb(InnerCallback(func)), frozen(0), stream(new WvStream)
    {
        stream->setcallback(WvStreamCallback(this, &WvDelayedCallback::thaw), 0);
        WvIStreamList::globallist.append(stream, true);
    }
    WvDelayedCallback(const WvDelayedCallback &other)
        : cb(other.cb), frozen(0), stream(new WvStream)
    {
        stream->setcallback(WvStreamCallback(this, &WvDelayedCallback::thaw), 0);
        WvIStreamList::globallist.append(stream, true);
    }
    ~WvDelayedCallback()
    {
        delete frozen;
    }
    R operator()()
    {
        frozen = new FrozenParams;
        stream->alarm(0);
        // you can't delay a callback that has a non-void return type, sorry
    }
    R operator()(P1 p1)
    {
        frozen = new FrozenParams(p1);
        stream->alarm(0);
    }
    R operator()(P1 p1, P2 p2)
    {
        frozen = new FrozenParams(p1, p2);
        stream->alarm(0);
    }
    R operator()(P1 p1, P2 p2, P3 p3)
    {
        frozen = new FrozenParams(p1, p2, p3);
        stream->alarm(0);
    }
    R operator()(P1 p1, P2 p2, P3 p3, P4 p4)
    {
        frozen = new FrozenParams(p1, p2, p3, p4);
        stream->alarm(0);
    }
    R operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    {
        frozen = new FrozenParams(p1, p2, p3, p4, p5);
        stream->alarm(0);
    }
    R operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
    {
        frozen = new FrozenParams(p1, p2, p3, p4, p5, p6);
        stream->alarm(0);
    }
    R operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7)
    {
        frozen = new FrozenParams(p1, p2, p3, p4, p5, p6, p7);
        stream->alarm(0);
    }
    R operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8)
    {
        frozen = new FrozenParams(p1, p2, p3, p4, p5, p6, p7, p8);
        stream->alarm(0);
    }
};
#endif
