/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2003 Net Integration Technologies, Inc.
 *
 */
#ifndef __WVDELAYEDCALLBACK_H
#define __WVDELAYEDCALLBACK_H

#include "wvistreamlist.h"
#include "wvtr1.h"

/**
 * A WvCallback wrapper that delays until the next tick of the WvIStreamList
 * main loop.
 *
 * There are restrictions on the type of the wrapped callback though:
 *   1. The return type must be void
 *   2. All parameter types must be copy-constructible value types
 */
template<class Functor>
class WvDelayedCallback
{
private:
    Functor func;
    WvStream *stream;
    wv::function<void()> frozen;

    void thaw()
    {
	assert(frozen);
	frozen();
	frozen = 0;
    }

public:
  WvDelayedCallback(const Functor& _func):
      func(_func), stream(new WvStream), frozen(0)
    {
        stream->setcallback(wv::bind(&WvDelayedCallback::thaw, this));
        WvIStreamList::globallist.append(stream, true, "WvDelayedCallback");
    }
    WvDelayedCallback(const WvDelayedCallback &other):
	func(other.func), stream(new WvStream), frozen(0)
    {
        stream->setcallback(wv::bind(&WvDelayedCallback::thaw, this));
        WvIStreamList::globallist.append(stream, true, "WvDelayedCallback");
    }
    ~WvDelayedCallback()
    {
        stream->setcallback(0);
        stream->close();
    }
    void operator()()
    {
        frozen = func;
        stream->alarm(0);
    }
    template<typename P1>
    void operator()(P1 &p1)
    {
	frozen = wv::bind(func, p1);
        stream->alarm(0);
    }
    template<typename P1,
	     typename P2>
    void operator()(P1 &p1, P2 &p2)
    {
        frozen = wv::bind(func, p1, p2);
        stream->alarm(0);
    }
    template<typename P1,
	     typename P2,
	     typename P3>
    void operator()(P1 &p1, P2 &p2, P3 &p3)
    {
        frozen = wv::bind(func, p1, p2, p3);
        stream->alarm(0);
    }
    template<typename P1,
	     typename P2,
	     typename P3,
	     typename P4>
    void operator()(P1 &p1, P2 &p2, P3 &p3, P4 &p4)
    {
        frozen = wv::bind(func, p1, p2, p3, p4);
        stream->alarm(0);
    }
    template<typename P1,
	     typename P2,
	     typename P3,
	     typename P4,
	     typename P5>
    void operator()(P1 &p1, P2 &p2, P3 &p3, P4 &p4, P5 &p5)
    {
        frozen = wv::bind(func, p1, p2, p3, p4, p5);
        stream->alarm(0);
    }
    template<typename P1,
	     typename P2,
	     typename P3,
	     typename P4,
	     typename P5,
	     typename P6>
    void operator()(P1 &p1, P2 &p2, P3 &p3, P4 &p4, P5 &p5, P6 &p6)
    {
        frozen = wv::bind(func, p1, p2, p3, p4, p5, p6);
        stream->alarm(0);
    }
    template<typename P1,
	     typename P2,
	     typename P3,
	     typename P4,
	     typename P5,
	     typename P6,
	     typename P7>
    void operator()(P1 &p1, P2 &p2, P3 &p3, P4 &p4, P5 &p5, P6 &p6, P7 &p7)
    {
        frozen = wv::bind(func, p1, p2, p3, p4, p5, p6, p7);
        stream->alarm(0);
    }
    template<typename P1,
	     typename P2,
	     typename P3,
	     typename P4,
	     typename P5,
	     typename P6,
	     typename P7,
	     typename P8>
    void operator()(P1 &p1, P2 &p2, P3 &p3, P4 &p4, P5 &p5, P6 &p6, P7 &p7,
		    P8 &p8)
    {
        frozen = wv::bind(func, p1, p2, p3, p4, p5, p6, p7, p8);
        stream->alarm(0);
    }
};

#endif
