/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 */
#ifndef __WVCALLBACK_H
#define __WVCALLBACK_H

class EmptyType;

template<typename R,
	 typename P1 = EmptyType,
	 typename P2 = EmptyType,
	 typename P3 = EmptyType,
	 typename P4 = EmptyType,
	 typename P5 = EmptyType,
	 typename P6 = EmptyType,
	 typename P7 = EmptyType,
	 typename P8 = EmptyType>
class WvCallbackImpl
{
public:
    typedef R(*type)(P1, P2, P3, P4, P5, P6, P7, P8);
    virtual R operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8) = 0;
    virtual WvCallbackImpl* clone() const = 0;
    virtual ~WvCallbackImpl()
        { }
};

template<typename R,
	 typename P1,
	 typename P2,
	 typename P3,
	 typename P4,
	 typename P5,
	 typename P6,
	 typename P7>
class WvCallbackImpl<R, P1, P2, P3, P4, P5, P6, P7, EmptyType>
{
public:
    typedef R(*type)(P1, P2, P3, P4, P5, P6, P7);
    virtual R operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7) = 0;
    virtual WvCallbackImpl* clone() const = 0;
    virtual ~WvCallbackImpl()
        { }
};

template<typename R,
	 typename P1,
	 typename P2,
	 typename P3,
	 typename P4,
	 typename P5,
	 typename P6>
class WvCallbackImpl<R, P1, P2, P3, P4, P5, P6, EmptyType, EmptyType>
{
public:
    typedef R(*type)(P1, P2, P3, P4, P5, P6);
    virtual R operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6) = 0;
    virtual WvCallbackImpl* clone() const = 0;
    virtual ~WvCallbackImpl()
        { }
};

template<typename R,
	 typename P1,
	 typename P2,
	 typename P3,
	 typename P4,
	 typename P5>
class WvCallbackImpl<R, P1, P2, P3, P4, P5, EmptyType, EmptyType, EmptyType>
{
public:
    typedef R(*type)(P1, P2, P3, P4, P5);
    virtual R operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) = 0;
    virtual WvCallbackImpl* clone() const = 0;
    virtual ~WvCallbackImpl()
        { }
};

template<typename R,
	 typename P1,
	 typename P2,
	 typename P3,
	 typename P4>
class WvCallbackImpl<R, P1, P2, P3, P4, EmptyType, EmptyType, EmptyType, EmptyType>
{
public:
    typedef R(*type)(P1, P2, P3, P4);
    virtual R operator()(P1 p1, P2 p2, P3 p3, P4 p4) = 0;
    virtual WvCallbackImpl* clone() const = 0;
    virtual ~WvCallbackImpl()
        { }
};

template<typename R,
	 typename P1,
	 typename P2,
	 typename P3>
class WvCallbackImpl<R, P1, P2, P3, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType>
{
public:
    typedef R(*type)(P1, P2, P3);
    virtual R operator()(P1 p1, P2 p2, P3 p3) = 0;
    virtual WvCallbackImpl* clone() const = 0;
    virtual ~WvCallbackImpl()
        { }
};

template<typename R,
	 typename P1,
	 typename P2>
class WvCallbackImpl<R, P1, P2, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType>
{
public:
    typedef R(*type)(P1, P2);
    virtual R operator()(P1 p1, P2 p2) = 0;
    virtual WvCallbackImpl* clone() const = 0;
    virtual ~WvCallbackImpl()
        { }
};

template<typename R,
	 typename P1>
class WvCallbackImpl<R, P1, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType>
{
public:
    typedef R(*type)(P1);
    virtual R operator()(P1 p1) = 0;
    virtual WvCallbackImpl* clone() const = 0;
    virtual ~WvCallbackImpl()
        { }
};

template<typename R>
class WvCallbackImpl<R, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType, EmptyType>
{
public:
    typedef R(*type)();
    virtual R operator()() = 0;
    virtual WvCallbackImpl* clone() const = 0;
    virtual ~WvCallbackImpl()
        { }
};

template<class ParentCallback,
	 typename Functor>
class WvCallbackFunctor
    : public WvCallbackImpl<typename ParentCallback::ReturnType,
			  typename ParentCallback::Parm1,
			  typename ParentCallback::Parm2,
			  typename ParentCallback::Parm3,
			  typename ParentCallback::Parm4,
			  typename ParentCallback::Parm5,
			  typename ParentCallback::Parm6,
			  typename ParentCallback::Parm7,
			  typename ParentCallback::Parm8>
{
    typedef typename ParentCallback::ReturnType R;
    typedef typename ParentCallback::Parm1 P1;
    typedef typename ParentCallback::Parm2 P2;
    typedef typename ParentCallback::Parm3 P3;
    typedef typename ParentCallback::Parm4 P4;
    typedef typename ParentCallback::Parm5 P5;
    typedef typename ParentCallback::Parm6 P6;
    typedef typename ParentCallback::Parm7 P7;
    typedef typename ParentCallback::Parm8 P8;
    Functor func;
public:
    WvCallbackFunctor(const Functor& _func): func(_func)
        { }
    WvCallbackFunctor* clone() const
        { return new WvCallbackFunctor(*this); }
    R operator()()
        { return func(); }
    R operator()(P1 p1)
        { return func(p1); }
    R operator()(P1 p1, P2 p2)
        { return func(p1, p2); }
    R operator()(P1 p1, P2 p2, P3 p3)
        { return func(p1, p2, p3); }
    R operator()(P1 p1, P2 p2, P3 p3, P4 p4)
        { return func(p1, p2, p3, p4); }
    R operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
        { return func(p1, p2, p3, p4, p5); }
    R operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
        { return func(p1, p2, p3, p4, p5, p6); }
    R operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7)
        { return func(p1, p2, p3, p4, p5, p6, p7); }
    R operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8)
        { return func(p1, p2, p3, p4, p5, p6, p7, p8); }
};

template<class ParentCallback,
	 typename PtrToObject,
	 typename PtrToMember>
class WvCallbackMember
    : public WvCallbackImpl<typename ParentCallback::ReturnType,
			  typename ParentCallback::Parm1,
			  typename ParentCallback::Parm2,
			  typename ParentCallback::Parm3,
			  typename ParentCallback::Parm4,
			  typename ParentCallback::Parm5,
			  typename ParentCallback::Parm6,
			  typename ParentCallback::Parm7,
			  typename ParentCallback::Parm8>
{
    typedef typename ParentCallback::ReturnType R;
    typedef typename ParentCallback::Parm1 P1;
    typedef typename ParentCallback::Parm2 P2;
    typedef typename ParentCallback::Parm3 P3;
    typedef typename ParentCallback::Parm4 P4;
    typedef typename ParentCallback::Parm5 P5;
    typedef typename ParentCallback::Parm6 P6;
    typedef typename ParentCallback::Parm7 P7;
    typedef typename ParentCallback::Parm8 P8;
    PtrToObject obj;
    PtrToMember member;
public:
    WvCallbackMember(PtrToObject _obj, PtrToMember _member)
	: obj(_obj), member(_member)
        { }
    WvCallbackMember* clone() const
        { return new WvCallbackMember(*this); }
    R operator()()
        { return ((*obj).*member)(); }
    R operator()(P1 p1)
        { return ((*obj).*member)(p1); }
    R operator()(P1 p1, P2 p2)
        { return ((*obj).*member)(p1, p2); }
    R operator()(P1 p1, P2 p2, P3 p3)
        { return ((*obj).*member)(p1, p2, p3); }
    R operator()(P1 p1, P2 p2, P3 p3, P4 p4)
        { return ((*obj).*member)(p1, p2, p3, p4); }
    R operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
        { return ((*obj).*member)(p1, p2, p3, p4, p5); }
    R operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
        { return ((*obj).*member)(p1, p2, p3, p4, p5, p6); }
    R operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7)
        { return ((*obj).*member)(p1, p2, p3, p4, p5, p6, p7); }
    R operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8)
        { return ((*obj).*member)(p1, p2, p3, p4, p5, p6, p7, p8); }
};

template<typename R,
	 typename P1 = EmptyType,
	 typename P2 = EmptyType,
	 typename P3 = EmptyType,
	 typename P4 = EmptyType,
	 typename P5 = EmptyType,
	 typename P6 = EmptyType,
	 typename P7 = EmptyType,
	 typename P8 = EmptyType>
class WvCallback
{
private:
    WvCallbackImpl<R, P1, P2, P3, P4, P5, P6, P7, P8>* impl;
public:
    typedef R ReturnType;
    typedef P1 Parm1;
    typedef P2 Parm2;
    typedef P3 Parm3;
    typedef P4 Parm4;
    typedef P5 Parm5;
    typedef P6 Parm6;
    typedef P7 Parm7;
    typedef P8 Parm8;
    WvCallback() : impl(0)
        { }
    WvCallback(int) : impl(0)
        { }
    WvCallback(const WvCallback& cb): impl(0)
        { if(cb.impl) impl = cb.impl->clone(); }
    template<typename Functor>
    WvCallback(const Functor& func)
        { impl = new WvCallbackFunctor<WvCallback, Functor>(func); }
    WvCallback(const typename WvCallbackImpl<R, P1, P2, P3, P4, P5, P6, P7, P8>::type func)
        { impl = new WvCallbackFunctor<WvCallback, 
		typename WvCallbackImpl<R, P1, P2, P3, P4, P5, P6, P7, P8>
		               ::type>(func); }
    template<typename PtrToObject, typename PtrToMember>
    WvCallback(PtrToObject obj, PtrToMember member)
        { impl = new WvCallbackMember<WvCallback, PtrToObject, PtrToMember>
		(obj, member); }
    ~WvCallback()
        { if(impl) delete impl; }
    
    WvCallback& operator=(const WvCallback& cb)
    {
	if(impl)
	{
	    delete impl;
	    impl = 0;
	}

	if(cb.impl)
	    impl = cb.impl->clone();

	return *this;
    }
    
    operator bool() const
        { return impl; }
    R operator()() const
        { return (*impl)(); }
    R operator()(P1 p1) const
        { return (*impl)(p1); }
    R operator()(P1 p1, P2 p2) const
        { return (*impl)(p1, p2); }
    R operator()(P1 p1, P2 p2, P3 p3) const
        { return (*impl)(p1, p2, p3); }
    R operator()(P1 p1, P2 p2, P3 p3, P4 p4) const
        { return (*impl)(p1, p2, p3, p4); }
    R operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) const
        { return (*impl)(p1, p2, p3, p4, p5); }
    R operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6) const
        { return (*impl)(p1, p2, p3, p4, p5, p6); }
    R operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7) const
        { return (*impl)(p1, p2, p3, p4, p5, p6, p7); }
    R operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8) const
        { return (*impl)(p1, p2, p3, p4, p5, p6, p7, p8); }
    
protected:
    // explicitly not defined: these callbacks are way to complex to
    // be explicitly compared!
    bool operator== (const WvCallback& cb);
};


template<class InnerCallback, typename B>
class WvBoundCallback
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
    WvCallback<R, B, P1, P2, P3, P4, P5, P6, P7> cb;
    B param;
public:
    template<typename PtrToObject, typename PtrToMember>
    WvBoundCallback(PtrToObject obj, PtrToMember member, const B _param)
        : cb(WvCallback<R, B, P1, P2, P3, P4, P5, P6, P7>(obj, member)),
          param(_param)
    { }
    template<typename Functor>
    WvBoundCallback(const Functor& func, const B _param)
        : cb(WvCallback<R, B, P1, P2, P3, P4, P5, P6, P7>(func)),
          param(_param)
    { }
    R operator()() const
        { return cb(param); }
    R operator()(P1 p1) const
        { return cb(param, p1); }
    R operator()(P1 p1, P2 p2) const
        { return cb(param, p1, p2); }
    R operator()(P1 p1, P2 p2, P3 p3) const
        { return cb(param, p1, p2, p3); }
    R operator()(P1 p1, P2 p2, P3 p3, P4 p4) const
        { return cb(param, p1, p2, p3, p4); }
    R operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) const
        { return cb(param, p1, p2, p3, p4, p5); }
    R operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6) const
        { return cb(param, p1, p2, p3, p4, p5, p6); }
    R operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7) const
        { return cb(param, p1, p2, p3, p4, p5, p6, p7); }
};

#endif /* __WVCALLBACK_H */
