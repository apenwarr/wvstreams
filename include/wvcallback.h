/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 */
#ifndef __WVCALLBACK_H
#define __WVCALLBACK_H

// the templated base class for WvCallback.  All the other callback classes
// come from this somehow.

// Fake is a boring object type that we use for calling our "generic"
// member function pointers.  Strangely, it crashes if Fake doesn't
// have a virtual table, so we have an empty virtual function to make it
// happy.  (This is all a bit evil, but only because C++ sucks.  When
// they pass me a callback, they _know_ which function I want to call; I
// don't need to resolve it at runtime...)
struct Fake { virtual void silly() {} };

template <class RET>
class WvCallbackBase
{
protected:
public:
    // FakeFunc is a completely generic member-function pointer.  Actually the
    // functions we _really_ call aren't part of the Fake class and probably
    // have different parameters, but some hideous typecasts should fix that
    // right up.
    typedef RET (Fake::*FakeFunc)();
    typedef RET (*FakeGlobalFunc)();
    
    Fake *obj;
    
    union {
	FakeFunc func;
	FakeGlobalFunc globalfunc;
    };
    
    WvCallbackBase(void *_obj, FakeFunc _func)
	: obj((Fake *)_obj), func(_func)
	{ }
    
    WvCallbackBase(FakeGlobalFunc _func)
	: obj(0), globalfunc(_func)
	{ }
    
    bool operator== (const WvCallbackBase &cb) const
        { if (obj)
	    return obj==cb.obj && func==cb.func;
	  else
	    return !cb.obj && globalfunc == cb.globalfunc;
	}
    
    operator bool () const
        { return obj || globalfunc; }
};


// Declare WvCallback#, an object derived from WvCallbackBase that takes
// n parameters.  This macro is mainly for use later in this header
// file, to avoid duplicated code.  It's supposed to be part of a template
// declaration - be careful!
#define __MakeWvCallback(n, decls, parms) \
    class WvCallback##n : public WvCallbackBase<RET> \
    { \
    protected: \
    public: \
        typedef typename WvCallbackBase<RET>::FakeFunc FakeFunc; \
        typedef typename WvCallbackBase<RET>::FakeGlobalFunc FakeGlobalFunc; \
	typedef RET (Fake::*Func) decls; \
	typedef RET (*GlobalFunc) decls; \
	WvCallback##n(Fake *_obj, Func _func) \
	    : WvCallbackBase<RET>(_obj, (FakeFunc)_func) { } \
	WvCallback##n(GlobalFunc _func) \
	    : WvCallbackBase<RET>((FakeGlobalFunc)_func) { } \
    public: \
	RET operator() decls const \
	    { \
	      if (obj) \
		return ((*obj).*(Func)func) parms; \
	      else \
		return ((GlobalFunc)globalfunc) parms; \
	    } \
    }

// Derive WvCallback#_bound, an actual instance of WvCallback# that has a
// particular object type.  This macro is mainly for use later in this header
// file, to avoid duplicated code.
// 
// Note: normally I don't like the silly C++ casting operators, but
// changing a normal typecast to reinterpret_cast<Func> below makes a _huge_
// improvement in code size.  (g++ 2.95.4)
#define __MakeWvBoundCallback(n, decls, basetype...) \
    class WvCallback##n##_bound : public basetype \
    { \
    public: \
        typedef typename basetype::Func Func; \
	typedef RET (T::*BoundFunc) decls; \
	WvCallback##n##_bound(T &_obj, BoundFunc _func) \
	    : basetype((Fake *)&_obj, reinterpret_cast<Func>(_func)) { } \
    }


// declare WvCallback# and WvCallback#_bound classes for 0 through 6
// parameters (we can add more parameters later, if necessary, by adding
// more declarations below)

template <class RET>
    __MakeWvCallback(0, (), ());
template <class RET, class T>
    __MakeWvBoundCallback(0, (), WvCallback0<RET>);

template <class RET, class P1>
    __MakeWvCallback(1, (P1 p1), (p1));
template <class RET, class T, class P1>
    __MakeWvBoundCallback(1, (P1 p1), WvCallback1<RET, P1>);

template <class RET, class P1, class P2>
    __MakeWvCallback(2, (P1 p1, P2 p2), (p1, p2));
template <class RET, class T, class P1, class P2>
    __MakeWvBoundCallback(2, (P1 p1, P2 p2), WvCallback2<RET, P1, P2>);

template <class RET, class P1, class P2, class P3>
    __MakeWvCallback(3, (P1 p1, P2 p2, P3 p3), (p1, p2, p3));
template <class RET, class T, class P1, class P2, class P3>
    __MakeWvBoundCallback(3, (P1 p1, P2 p2, P3 p3),
			  WvCallback3<RET, P1, P2, P3>);

template <class RET, class P1, class P2, class P3, class P4>
    __MakeWvCallback(4, (P1 p1, P2 p2, P3 p3, P4 p4), (p1, p2, p3, p4));
template <class RET, class T, class P1, class P2, class P3, class P4>
    __MakeWvBoundCallback(4, (P1 p1, P2 p2, P3 p3, P4 p4),
			  WvCallback4<RET, P1, P2, P3, P4>);

template <class RET, class P1, class P2, class P3, class P4, class P5>
    __MakeWvCallback(5, (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5),
		     (p1, p2, p3, p4, p5));
template <class RET, class T, class P1, class P2, class P3, class P4, class P5>
    __MakeWvBoundCallback(5, (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5),
			  WvCallback5<RET, P1, P2, P3, P4, P5>);

template <class RET, class P1, class P2, class P3, class P4, class P5, class P6>
    __MakeWvCallback(6, (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6),
		     (p1, p2, p3, p4, p5, p6));
template <class RET, class T, class P1, class P2, class P3, class P4, class P5,
    		class P6>
    __MakeWvBoundCallback(6, (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6),
			  WvCallback6<RET, P1, P2, P3, P4, P5, P6>);


// DeclareWvCallback is how you create a new type of callback.  The options
// are:
//    n - the number of parameters the callback takes (only 0..4 are allowed)
//    ret - the return value of the callback
//    type - the name of the callback type
//    parms... - the types of the parameters.  There are 'n' of these.
// 
// Example:
//     DeclareWvCallback(3, void, TriCallback, Object*, Blob&, bool);
//     
// NOTE!!
// 
// For some reason, running this inside a class definition causes an internal
// compiler error (g++ 2.95.4).  So you'll have to declare your callbacks
// at the top level, rather than inside your class where it might seem to
// make more sense.  Oh well, at least it's less typing that way...
// 
#define DeclareWvCallback(n, ret, type, parms...) \
    typedef WvCallback##n<ret , ## parms> type; \
    \
    template <class T> \
	class type##_bound : public WvCallback##n##_bound<ret,T , ## parms> \
	{ \
	public: \
            typedef typename \
                WvCallback##n##_bound<ret,T , ## parms>::BoundFunc BoundFunc; \
	    type##_bound(T &_obj, BoundFunc _func) \
		: WvCallback##n##_bound<ret,T , ## parms>(_obj, _func) {} \
	}

// Use wvcallback() to actually generate a new callback object, given the
// callback type, object type, and member function.  The callback type needs
// to have been declared earlier with DeclareWvCallback.
// 
// Example:
//     CallBackType cb = wvcallback(CallbackType, myobject, MyObject::Func);
//     
// You can pass the result of this call to anything that's expecting a
// callback of the appropriate type; you don't need to actually create the
// object first.
// 
#define wvcallback(cbname, instance, func) \
    cbname##_bound<typeof(instance)>(instance, &func)


// Some types of callbacks are so common we'll just declare them here.

DeclareWvCallback(0, void, VoidCallback);

#endif // __WVCALLBACK_H
