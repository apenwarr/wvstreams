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
    WvMap<void*, InnerCallback> list;

    /* The idea of copying this gives me a headache. */
    WvCallbackList(const WvCallbackList &);
    WvCallbackList& operator=(const WvCallbackList&);
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
    bool isempty() const
    {
	return list.isempty();
    }
    void operator()()
    {
	typename WvMap<void*, InnerCallback>::Iter i(list);

	for (i.rewind(); i.next(); )
	    i().data();
    }
    template<typename P1>
    void operator()(P1 &p1)
    {
	typename WvMap<void*, InnerCallback>::Iter i(list);

	for (i.rewind(); i.next(); )
	    i().data(p1);
    }
    template<typename P1,
	     typename P2>
    void operator()(P1 &p1, P2 &p2)
    {
	typename WvMap<void*, InnerCallback>::Iter i(list);

	for (i.rewind(); i.next(); )
	    i().data(p1, p2);
    }
    template<typename P1,
	     typename P2,
	     typename P3>
    void operator()(P1 &p1, P2 &p2, P3 &p3)
    {
	typename WvMap<void*, InnerCallback>::Iter i(list);

	for (i.rewind(); i.next(); )
	    i().data(p1, p2, p3);
    }
    template<typename P1,
	     typename P2,
	     typename P3,
	     typename P4>
    void operator()(P1 &p1, P2 &p2, P3 &p3, P4 &p4)
    {
	typename WvMap<void*, InnerCallback>::Iter i(list);

	for (i.rewind(); i.next(); )
	    i().data(p1, p2, p3, p4);
    }
    template<typename P1,
	     typename P2,
	     typename P3,
	     typename P4,
	     typename P5>
    void operator()(P1 &p1, P2 &p2, P3 &p3, P4 &p4, P5 &p5)
    {
	typename WvMap<void*, InnerCallback>::Iter i(list);

	for (i.rewind(); i.next(); )
	    i().data(p1, p2, p3, p4, p5);
    }
    template<typename P1,
	     typename P2,
	     typename P3,
	     typename P4,
	     typename P5,
	     typename P6>
    void operator()(P1 &p1, P2 &p2, P3 &p3, P4 &p4, P5 &p5, P6 &p6)
    {
	typename WvMap<void*, InnerCallback>::Iter i(list);

	for (i.rewind(); i.next(); )
	    i().data(p1, p2, p3, p4, p5, p6);
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
	typename WvMap<void*, InnerCallback>::Iter i(list);

	for (i.rewind(); i.next(); )
	    i().data(p1, p2, p3, p4, p5, p6, p7);
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
	typename WvMap<void*, InnerCallback>::Iter i(list);

	for (i.rewind(); i.next(); )
	    i().data(p1, p2, p3, p4, p5, p6, p7, p8);
    }
};


#endif /* __WVCALLBACKLIST_H */
