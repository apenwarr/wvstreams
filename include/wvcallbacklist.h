/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 */
#ifndef __WVCALLBACKLIST_H
#define __WVCALLBACKLIST_H


#include <wvhashtable.h>


template<class InnerCallback>
class WvCallbackList
{
private:
    typedef typename InnerCallback::Parm1 P1;
    typedef typename InnerCallback::Parm2 P2;
    typedef typename InnerCallback::Parm3 P3;
    typedef typename InnerCallback::Parm4 P4;
    typedef typename InnerCallback::Parm5 P5;
    typedef typename InnerCallback::Parm6 P6;
    typedef typename InnerCallback::Parm7 P7;
    typedef typename InnerCallback::Parm8 P8;
    WvMap<void*, InnerCallback> list;

    /* The idea of copying this gives me a headache. */
    WvCallbackList(const WvCallbackList &);
public:
    WvCallbackList():
	list(42)
    {
    }
    void add(const InnerCallback &cb, void *cookie)
    {
	assert(!list.exists(cookie));
	list.add(cookie, cb);
    }
    void del(void *cookie)
    {
	assert(list.exists(cookie));
	list.remove(cookie);
    }
    void operator()()
    {
	WvMap<void*, InnerCallback>::Iter i(list);

	for (i.rewind(); i.next(); )
	    i().data();
    }
    void operator()(P1 p1)
    {
	WvMap<void*, InnerCallback>::Iter i(list);

	for (i.rewind(); i.next(); )
	    i().data(p1);
    }
    void operator()(P1 p1, P2 p2)
    {
	WvMap<void*, InnerCallback>::Iter i(list);

	for (i.rewind(); i.next(); )
	    i().data(p1, p2);
    }
    void operator()(P1 p1, P2 p2, P3 p3)
    {
	WvMap<void*, InnerCallback>::Iter i(list);

	for (i.rewind(); i.next(); )
	    i().data(p1, p2, p3);
    }
    void operator()(P1 p1, P2 p2, P3 p3, P4 p4)
    {
	WvMap<void*, InnerCallback>::Iter i(list);

	for (i.rewind(); i.next(); )
	    i().data(p1, p2, p3, p4);
    }
    void operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    {
	WvMap<void*, InnerCallback>::Iter i(list);

	for (i.rewind(); i.next(); )
	    i().data(p1, p2, p3, p4, p5);
    }
    void operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
    {
	WvMap<void*, InnerCallback>::Iter i(list);

	for (i.rewind(); i.next(); )
	    i().data(p1, p2, p3, p4, p5, p6);
    }
    void operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7)
    {
	WvMap<void*, InnerCallback>::Iter i(list);

	for (i.rewind(); i.next(); )
	    i().data(p1, p2, p3, p4, p5, p6, p7);
    }
    void operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8)
    {
	WvMap<void*, InnerCallback>::Iter i(list);

	for (i.rewind(); i.next(); )
	    i().data(p1, p2, p3, p4, p5, p6, p7, p8);
    }
};


#endif /* __WVCALLBACKLIST_H */
