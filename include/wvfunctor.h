/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 *
 * Declares a smart bound functor type.
 * 
 * Inspired from ideas presented in Modern C++ Design, Generic
 * Programming and Design Patterns Applied, by Andrei Alexandrescu.
 *
 * Key differences from the material presented in that book:
 *   - Null functors are supported.
 *   - Clients need not know anything about TypeLists because the
 *     WvFunctor<> template uses a long list of individual argument
 *     types with default values rather than a TypeList.
 *   - Clients may bind arguments directly in the constructor
 *     without the need for explicit BindFirst<> calls.
 *   - Pointers to member functions generate Functors whose first
 *     argument must be a pointer to an object of that type.
 *   - FunctorInfo is used to pass around types rather than requiring
 *     both the return type and argument type list or a specialized
 *     Functor type to be specified.
 *   - FunctorImplChooser explicitly specified what type of FunctorImpl
 *     is to be instantiated to handle each type of functor.  This
 *     permits us to be more clever with pointers to member functions.
 *   - No dependence on the C++ STL.
 */
#ifndef __WVFUNCTOR_H
#define __WVFUNCTOR_H

#include "wvtypelist.h"

/**
 * If this macro is defined, enables a some ugly hacks for getting
 * around missing RTTI support.
 */
#define NO_RTTI

namespace WvGeneric
{

#ifdef NO_RTTI

/**
 * @internal
 * Returns true if the instances t and s have compatible types.
 * <p>
 * When RTTI support is not enabled, this merely checks for type
 * equality, not strict compatibility.
 * </p>
 */
template<class T, class S>
inline T* check_type(T* t, S* s)
{
    return t->type() == s->type() ? static_cast<T*>(s) : 0;
}
#define DECLARE_TYPE \
virtual int type() const { \
    static int TYPE = ++PREVTYPE; \
    return TYPE; \
}

#else
/**
 * @internal
 * Returns true if the instances t and s have compatible types.
 * <p>
 * When RTTI support is not enabled, this merely checks for type
 * equality, not strict compatibility.
 * </p>
 */
template<class T, class S>
inline T* check_type(T* t, S* s)
{
    return dynamic_cast<T*>(s);
}
#define DECLARE_TYPE

#endif
   

/**
 * @internal
 * The FunctorInfo template describes properties of an abstract
 * functor, including its parameter and return types.
 *
 * @param R the functor return type
 * @param TL the functor argument type list
 */
template<class R, class TL>
struct FunctorInfo
{
    typedef R Ret;
    typedef TL Args;
    typedef typename TypeAt<TL, 0>::Result Arg1;
    typedef typename TypeAt<TL, 1>::Result Arg2;
    typedef typename TypeAt<TL, 2>::Result Arg3;
    typedef typename TypeAt<TL, 3>::Result Arg4;
    typedef typename TypeAt<TL, 4>::Result Arg5;
    typedef typename TypeAt<TL, 5>::Result Arg6;
};


/**
 * @internal
 * The FunctorImplBase class just defines the virtual clone and
 * destructor, so we can later build a FunctorBase defined in terms
 * of these methods to minimize the amount of code generated.
 *
 * Cloning is provided to support copy semantics.
 *
 * @see FunctorBase
 */
class FunctorImplBase
{
public:
    virtual ~FunctorImplBase();

    /**
     * Returns a new clone of this functor implementation.
     * @return the clone, non-null
     */
    virtual FunctorImplBase* clone() const = 0;

    /**
     * Returns true if the functors are equal.
     * <p>
     * Considers the deep structure of both objects.  Functors may
     * be considered non-equal because their structures differ
     * due to their manner of construction such as the order by
     * which arguments may have been bound.
     * </p>
     * @param other the functor to compare against
     * @return true if equal
     */
    virtual bool equals(const FunctorImplBase& other) const = 0;
    
#ifdef NO_RTTI
    /**
     * @internal
     * Returns a unique dynamically assigned type code to identify
     * objects of this class when RTTI support is not enabled.
     * @return a unique type code
     */
    virtual int type() const = 0;

protected:
    /**
     * @internal
     * The most recently issued type code.
     */
    static int PREVTYPE;
#endif
};


/**
 * @internal
 * The FunctorImpl template represents the base class for
 * implementations of function call dispatchers.
 *
 * Partial template instantiations are used to ensure that the
 * correct virtual operator() is created.
 *
 * @param Info the corresponding FunctorInfo type
 */
template<class Info>
class FunctorImpl;

// define FunctorImplBase for each number of arguments
#define DECLARE_FUNCTORIMPL(_n_, _args_...) \
class FunctorImpl<FunctorInfo<R, TYPELIST_##_n_ (_args_ ) > > : \
    public FunctorImplBase \
{ \
public: \
    virtual R operator()(_args_) const = 0; \
};

template<class R>
DECLARE_FUNCTORIMPL(X, )

template<class R, class A1>
DECLARE_FUNCTORIMPL(1, A1)

template<class R, class A1, class A2>
DECLARE_FUNCTORIMPL(2, A1, A2)

template<class R, class A1, class A2, class A3>
DECLARE_FUNCTORIMPL(3, A1, A2, A3)

template<class R, class A1, class A2, class A3, class A4>
DECLARE_FUNCTORIMPL(4, A1, A2, A3, A4)

template<class R, class A1, class A2, class A3, class A4, class A5>
DECLARE_FUNCTORIMPL(5, A1, A2, A3, A4, A5)

template<class R, class A1, class A2, class A3, class A4, class A5,
    class A6>
DECLARE_FUNCTORIMPL(6, A1, A2, A3, A4, A5, A6)

#undef DECLARE_FUNCTORIMPL

/**
 * @internal
 * The FunctorHandler template implements FunctorImpl to invoke a
 * generic functor, ie any object that declares an operator().
 *
 * Note that operator() is overloaded for each number of arguments,
 * but since only the correct operator() is declared virtual in
 * the base class, none of the others will every be instantiated.
 * This trick saves us a lot of typing.
 *
 * @param Info the corresponding FunctorInfo type
 * @param Fun the functor class type, or function pointer type
 */
template<class Info, class Fun>
class FunctorHandler : public FunctorImpl<Info>
{
    mutable Fun fun;

public:
    inline explicit FunctorHandler(const Fun& _fun) :
        fun(_fun) { }

    inline static FunctorHandler *create(const Fun& _fun)
    {
        return new FunctorHandler(_fun);
    }

    virtual FunctorHandler* clone() const
    {
        return new FunctorHandler(*this);
    }
    virtual bool equals(const FunctorImplBase& other) const
    {
        const FunctorHandler *o = check_type(this, & other);
        return o && fun == o->fun;
    }
    DECLARE_TYPE
    
    // only one of these will ever be instantiated
    typename Info::Ret operator()() const
    {
        return fun();
    }
    typename Info::Ret operator()(typename Info::Arg1 a1) const
    {
        return fun(a1);
    }
    typename Info::Ret operator()(typename Info::Arg1 a1,
        typename Info::Arg2 a2) const
    {
        return fun(a1, a2);
    }
    typename Info::Ret operator()(typename Info::Arg1 a1,
        typename Info::Arg2 a2, typename Info::Arg3 a3) const
    {
        return fun(a1, a2, a3);
    }
    typename Info::Ret operator()(typename Info::Arg1 a1,
        typename Info::Arg2 a2, typename Info::Arg3 a3,
        typename Info::Arg4 a4) const
    {
        return fun(a1, a2, a3, a4);
    }
    typename Info::Ret operator()(typename Info::Arg1 a1,
        typename Info::Arg2 a2, typename Info::Arg3 a3,
        typename Info::Arg4 a4, typename Info::Arg5 a5) const
    {
        return fun(a1, a2, a3, a4, a5);
    }
    typename Info::Ret operator()(typename Info::Arg1 a1,
        typename Info::Arg2 a2, typename Info::Arg3 a3,
        typename Info::Arg4 a4, typename Info::Arg5 a5,
        typename Info::Arg6 a6) const
    {
        return fun(a1, a2, a3, a4, a5, a6);
    }
};


/**
 * @internal
 * The StaticFunHandler template implements FunctorImpl to invoke a
 * static (global) function through a function pointer.
 * 
 * To reduce generated code size, all function pointer types are
 * treated in the same way with some ugly casts to perform the
 * necessary conversions.
 *
 * @param Info the corresponding FunctorInfo type
 */
template<class Info>
class StaticFunHandler : public FunctorImpl<Info>
{
    typedef typename Info::Ret (*FakeFun)();
    FakeFun fun;

public:
    inline explicit StaticFunHandler(const FakeFun& _fun) :
        fun(_fun) { }
        
    template<class Fun>
    inline static StaticFunHandler *create(const Fun& _fun)
    {
        return _fun ? new StaticFunHandler(
            reinterpret_cast<const FakeFun&>(_fun)) : 0;
    }

    virtual StaticFunHandler* clone() const
    {
        return new StaticFunHandler(*this);
    }
    virtual bool equals(const FunctorImplBase& other) const
    {
        const StaticFunHandler *o = check_type(this, & other);
        return o && fun == o->fun;
    }
    DECLARE_TYPE
    
    // only one of these will ever be instantiated
    typename Info::Ret operator()() const
    {
        return fun();
    }
    typename Info::Ret operator()(typename Info::Arg1 a1) const
    {
        return reinterpret_cast<typename Info::Ret (*)(
            typename Info::Arg1)>(fun)(
            a1);
    }
    typename Info::Ret operator()(typename Info::Arg1 a1,
        typename Info::Arg2 a2) const
    {
        return reinterpret_cast<typename Info::Ret (*)(
            typename Info::Arg1, typename Info::Arg2)>(fun)(
            a1, a2);
    }
    typename Info::Ret operator()(typename Info::Arg1 a1,
        typename Info::Arg2 a2, typename Info::Arg3 a3) const
    {
        return reinterpret_cast<typename Info::Ret (*)(
            typename Info::Arg1, typename Info::Arg2,
            typename Info::Arg3)>(fun)(
            a1, a2, a3);
    }
    typename Info::Ret operator()(typename Info::Arg1 a1,
        typename Info::Arg2 a2, typename Info::Arg3 a3,
        typename Info::Arg4 a4) const
    {
        return reinterpret_cast<typename Info::Ret (*)(
            typename Info::Arg1, typename Info::Arg2,
            typename Info::Arg3, typename Info::Arg4)>(fun)(
            a1, a2, a3, a4);
    }
    typename Info::Ret operator()(typename Info::Arg1 a1,
        typename Info::Arg2 a2, typename Info::Arg3 a3,
        typename Info::Arg4 a4, typename Info::Arg5 a5) const
    {
        return reinterpret_cast<typename Info::Ret (*)(
            typename Info::Arg1, typename Info::Arg2,
            typename Info::Arg3, typename Info::Arg4,
            typename Info::Arg5)>(fun)(
            a1, a2, a3, a4, a5);
    }
    typename Info::Ret operator()(typename Info::Arg1 a1,
        typename Info::Arg2 a2, typename Info::Arg3 a3,
        typename Info::Arg4 a4, typename Info::Arg5 a5,
        typename Info::Arg6 a6) const
    {
        return reinterpret_cast<typename Info::Ret (*)(
            typename Info::Arg1, typename Info::Arg2,
            typename Info::Arg3, typename Info::Arg4,
            typename Info::Arg5, typename Info::Arg6)>(fun)(
            a1, a2, a3, a4, a5, a6);
    }
};
    

/**
 * @internal
 * The MemFunHandler template implements FunctorImpl to invoke a
 * member function or const member function on an object.
 *
 * The first argument of a function call must always be a pointer
 * to the object whose member function is to be invoked.
 *
 * To reduce generated code size, all function pointer types are
 * treated in the same way with some ugly casts to perform the
 * necessary conversions.
 *
 * @param Info the corresponding FunctorInfo type
 */
template<class Info>
class MemFunHandler : public FunctorImpl<Info>
{
    struct Fake { virtual ~Fake() { } };
    typedef typename Info::Ret (Fake::*FakeFun)();

    FakeFun fun;

public:
    inline explicit MemFunHandler(const FakeFun& _fun) :
        fun(_fun) { }
        
    template<class Fun>
    inline static MemFunHandler *create(const Fun& _fun)
    {
        return _fun ? new MemFunHandler(
            reinterpret_cast<const FakeFun&>(_fun)) : 0;
    }

    virtual MemFunHandler* clone() const
    {
        return new MemFunHandler(*this);
    }
    virtual bool equals(const FunctorImplBase& other) const
    {
        const MemFunHandler *o = check_type(this, & other);
        return o && fun == o->fun;
    }
    DECLARE_TYPE
    
    // only one of these will ever be instantiated
    typename Info::Ret operator()(typename Info::Arg1 a1) const
    {
        return (reinterpret_cast<Fake*>(a1)->*fun)();
    }
    typename Info::Ret operator()(typename Info::Arg1 a1,
        typename Info::Arg2 a2) const
    {
        return (reinterpret_cast<Fake*>(a1)->*
            reinterpret_cast<typename Info::Ret (Fake::*)(
                typename Info::Arg2)>(fun))(a2);
    }
    typename Info::Ret operator()(typename Info::Arg1 a1,
        typename Info::Arg2 a2, typename Info::Arg3 a3) const
    {
        return (reinterpret_cast<Fake*>(a1)->*
            reinterpret_cast<typename Info::Ret (Fake::*)(
                typename Info::Arg2, typename Info::Arg3)>(fun))(
                a2, a3);
    }
    typename Info::Ret operator()(typename Info::Arg1 a1,
        typename Info::Arg2 a2, typename Info::Arg3 a3,
        typename Info::Arg4 a4) const
    {
        return (reinterpret_cast<Fake*>(a1)->*
            reinterpret_cast<typename Info::Ret (Fake::*)(
                typename Info::Arg2, typename Info::Arg3,
                typename Info::Arg4)>(fun))(
                a2, a3, a4);
    }
    typename Info::Ret operator()(typename Info::Arg1 a1,
        typename Info::Arg2 a2, typename Info::Arg3 a3,
        typename Info::Arg4 a4, typename Info::Arg5 a5) const
    {
        return (reinterpret_cast<Fake*>(a1)->*
            reinterpret_cast<typename Info::Ret (Fake::*)(
                typename Info::Arg2, typename Info::Arg3,
                typename Info::Arg4, typename Info::Arg5)>(fun))(
                a2, a3, a4, a5);
    }
    typename Info::Ret operator()(typename Info::Arg1 a1,
        typename Info::Arg2 a2, typename Info::Arg3 a3,
        typename Info::Arg4 a4, typename Info::Arg5 a5,
        typename Info::Arg6 a6) const
    {
        return (reinterpret_cast<Fake*>(a1)->*
            reinterpret_cast<typename Info::Ret (Fake::*)(
                typename Info::Arg2, typename Info::Arg3,
                typename Info::Arg4, typename Info::Arg5,
                typename Info::Arg6)>(fun))(
                a2, a3, a4, a5, a6);
    }
};


/**
 * @internal
 * The Binder1stHandler template implements FunctorImpl to invoke a
 * functor with the first argument bound to a particular value.
 *
 * @param Info the corresponding FunctorInfo type
 * @param Bound the bound argument type
 */
template<class Info, class Bound>
class Bind1stHandler : public FunctorImpl<Info>
{
public:
    typedef FunctorInfo<typename Info::Ret,
        TypeList<Bound, typename Info::Args> > BoundInfo;
    typedef FunctorImpl<BoundInfo> BoundFunctorImpl;

private:
    BoundFunctorImpl *fun;
    Bound bound;
    
public:
    inline Bind1stHandler(BoundFunctorImpl* fun, Bound bound) :
        fun(fun), bound(bound) { }

    virtual Bind1stHandler* clone() const
    {
        return new Bind1stHandler(static_cast<BoundFunctorImpl*>(
            fun->clone()), bound);
    }
    virtual bool equals(const FunctorImplBase& other) const
    {
        const Bind1stHandler *o = check_type(this, & other);
        return o && fun->equals(*o->fun) && bound == o->bound;
    }
    DECLARE_TYPE
    
    // only one of these will ever be instantiated
    typename Info::Ret operator()() const
    {
        return (*fun)(bound);
    }
    typename Info::Ret operator()(typename Info::Arg1 a1) const
    {
        return (*fun)(bound, a1);
    }
    typename Info::Ret operator()(typename Info::Arg1 a1,
        typename Info::Arg2 a2) const
    {
        return (*fun)(bound, a1, a2);
    }
    typename Info::Ret operator()(typename Info::Arg1 a1,
        typename Info::Arg2 a2, typename Info::Arg3 a3) const
    {
        return (*fun)(bound, a1, a2, a3);
    }
    typename Info::Ret operator()(typename Info::Arg1 a1,
        typename Info::Arg2 a2, typename Info::Arg3 a3,
        typename Info::Arg4 a4) const
    {
        return (*fun)(bound, a1, a2, a3, a4);
    }
    typename Info::Ret operator()(typename Info::Arg1 a1,
        typename Info::Arg2 a2, typename Info::Arg3 a3,
        typename Info::Arg4 a4, typename Info::Arg5 a5) const
    {
        return (*fun)(bound, a1, a2, a3, a4, a5);
    }
    typename Info::Ret operator()(typename Info::Arg1 a1,
        typename Info::Arg2 a2, typename Info::Arg3 a3,
        typename Info::Arg4 a4, typename Info::Arg5 a5,
        typename Info::Arg6 a6) const
    {
        return (*fun)(bound, a1, a2, a3, a4, a5, a6);
    }
};


/**
 * @internal
 * Binds the first argument of a functor and returns an
 * instance of a suitable bound functor implementation.
 */
template<class Bound, class BoundInfo>
inline FunctorImpl<FunctorInfo<typename BoundInfo::Ret,
    typename BoundInfo::Args::Tail> > *bind1st(
    FunctorImpl<BoundInfo> *fun, Bound bound)
{
    return new Bind1stHandler<FunctorInfo<typename BoundInfo::Ret,
        typename BoundInfo::Args::Tail>, Bound>(fun, bound);
}


/**
 * @internal
 * The FunctorImplChooser matches the supplied functor type with
 * a suitable implementation of FunctorImpl.
 *
 * @param Info the FunctorInfo type for the functor
 * @param Fun the functor type
 */
template<class Info, class Fun>
struct FunctorImplChooser
{
    // default case, assume Fun is an abstract functor class
    typedef FunctorHandler<Info, Fun> Result;
};

// rules for const and non-const member function pointers
#define DECLARE_MEMFUN_FUNCTORIMPLCHOOSER(_const_, _args_...) \
struct FunctorImplChooser<Info, R (T::*)(_args_) _const_> \
{ \
    typedef MemFunHandler<Info> Result; \
};
// rules for static (global) function pointers
#define DECLARE_STATICFUN_FUNCTORIMPLCHOOSER(_args_...) \
struct FunctorImplChooser<Info, R (*)(_args_)> \
{ \
    typedef StaticFunHandler<Info> Result; \
};

template<class Info, class R>
DECLARE_STATICFUN_FUNCTORIMPLCHOOSER( )

template<class Info, class R, class T>
DECLARE_MEMFUN_FUNCTORIMPLCHOOSER(, )
template<class Info, class R, class T>
DECLARE_MEMFUN_FUNCTORIMPLCHOOSER(const, )
template<class Info, class R, class A1>
DECLARE_STATICFUN_FUNCTORIMPLCHOOSER(A1)

template<class Info, class R, class T, class A1>
DECLARE_MEMFUN_FUNCTORIMPLCHOOSER(, A1)
template<class Info, class R, class T, class A1>
DECLARE_MEMFUN_FUNCTORIMPLCHOOSER(const, A1)
template<class Info, class R, class A1, class A2>
DECLARE_STATICFUN_FUNCTORIMPLCHOOSER(A1, A2)

template<class Info, class R, class T, class A1, class A2>
DECLARE_MEMFUN_FUNCTORIMPLCHOOSER(, A1, A2)
template<class Info, class R, class T, class A1, class A2>
DECLARE_MEMFUN_FUNCTORIMPLCHOOSER(const, A1, A2)
template<class Info, class R, class A1, class A2, class A3>
DECLARE_STATICFUN_FUNCTORIMPLCHOOSER(A1, A2, A3)

template<class Info, class R, class T, class A1, class A2, class A3>
DECLARE_MEMFUN_FUNCTORIMPLCHOOSER(, A1, A2, A3)
template<class Info, class R, class T, class A1, class A2, class A3>
DECLARE_MEMFUN_FUNCTORIMPLCHOOSER(const, A1, A2, A3)
template<class Info, class R, class A1, class A2, class A3, class A4>
DECLARE_STATICFUN_FUNCTORIMPLCHOOSER(A1, A2, A3, A4)

template<class Info, class R, class T, class A1, class A2, class A3,
    class A4>
DECLARE_MEMFUN_FUNCTORIMPLCHOOSER(, A1, A2, A3, A4)
template<class Info, class R, class T, class A1, class A2, class A3,
    class A4>
DECLARE_MEMFUN_FUNCTORIMPLCHOOSER(const, A1, A2, A3, A4)
template<class Info, class R, class A1, class A2, class A3, class A4,
    class A5>
DECLARE_STATICFUN_FUNCTORIMPLCHOOSER(A1, A2, A3, A4, A5)

template<class Info, class R, class T, class A1, class A2, class A3,
    class A4, class A5>
DECLARE_MEMFUN_FUNCTORIMPLCHOOSER(, A1, A2, A3, A4, A5)
template<class Info, class R, class T, class A1, class A2, class A3,
    class A4, class A5>
DECLARE_MEMFUN_FUNCTORIMPLCHOOSER(const, A1, A2, A3, A4, A5)
template<class Info, class R, class A1, class A2, class A3, class A4,
    class A5, class A6>
DECLARE_STATICFUN_FUNCTORIMPLCHOOSER(A1, A2, A3, A4, A5, A6)

#undef DECLARE_MEMFUN_FUNCTORIMPLCHOOSER
#undef DECLARE_STATICFUN_FUNCTORIMPLCHOOSER

/**
 * @internal
 * The FunctorBase class just defines the common storage
 * management semantics of WvFunctor to minimize generated code.
 */
class FunctorBase
{
protected:
    /**
     * The function call dispatch implementation.
     */
    FunctorImplBase *impl;

    /**
     * Creates a null functor.
     */
    inline explicit FunctorBase() :
        impl(0) { }

    /**
     * Creates a functor.
     * @param _impl the dispatch implementation
     */
    inline explicit FunctorBase(FunctorImplBase *_impl) :
        impl(_impl) { }

    /**
     * Creates a functor.
     * @param other the functor to be copied
     */
    explicit FunctorBase(const FunctorBase &other);

    /**
     * Assigns the dispatch implementation.
     * @param other the functor to be copied
     */
    void set(const FunctorBase& other);

    /**
     * Assigns the dispatch implementation to null.
     */
    void clear();

    /**
     * Returns true if the functors are the same.
     */
    bool equals(const FunctorBase& other) const;

public:
    /**
     * Destroys the functor and its dispatch implementation.
     */
    ~FunctorBase();
    
    /**
     * Determines whether the functor is null.
     * @return true if the functor is null.
     */
    inline bool operator!() const {
        return ! impl;
    }
};


/**
 * The WvFunctor template wraps a FunctorImpl and provides creation,
 * binding, copying, invocation, null-testing and comparison semantics.
 *
 * Argument values may be bound directly in the constructor,
 * allowing a functor with many arguments to be transformed into
 * one with fewer.  The values supplied are bound to arguments
 * in sequence beginning with the first one in the argument list.
 *
 * Binding of arguments from last to first or in arbitrary order
 * is not currently supported.
 *
 * The following C++ functor types may be wrapped:
 * <ul>
 * <li>A pointer to a static or global function.</li>
 * <li>A pointer to a member function.  The first argument is taken
 *     to be a pointer to an instance of the object that defines the
 *     member function.  Often this argument is bound to an instance
 *     when the object is created.</li>
 * <li>Any other type that defines an operator().</li>
 * </ul>
 *
 *
 * <em>Example:</em>
 * <pre>
 * class Test {
 *     // create function pointer member
 *     WvFunctor<void, int> ptr;
 *
 *     // declare callback
 *     void defcallback(const char* str, int status);
 *
 * public:
 *     // initialize the pointer to the default callback bound
 *     // to this object and a debugging string
 *     Test() : ptr(&Test::defcallback, this, "bound to const char*") { }
 *
 *     // let somebody else change it
 *     void setcallback(WvFunctor<void, int> newptr) {
 *         ptr = newptr;
 *     }
 *
 *     // use it
 *     void dosomething() {
 *         // null test
 *         if (!! ptr)
 *         {
 *             // invocation
 *             ptr(42); // eg. by default, defcallback("default", 42)
 *         }
 *         // comparison
 *         if (ptr == WvFunctor<void, int>(&Test::defcallback, this,
 *             "bound to const char*"))
 *             puts("Boring!");
 *     }
 *
 *     // clear it
 *     void clearcallback() {
 *         ptr = 0;
 *     }
 * };
 * </pre>
 *
 * @param R the return type, possibly void
 * @param A1 the first argument type, or NullType if none
 * @param A2 the second argument type, or NullType if none
 * @param A3 the third argument type, or NullType if none
 * @param A4 the fourth argument type, or NullType if none
 * @param A5 the fifth argument type, or NullType if none
 * @param A6 the sixth argument type, or NullType if none
 */
template<class R,
    class A1=NullType, class A2=NullType,
    class A3=NullType, class A4=NullType,
    class A5=NullType, class A6=NullType>
class WvFunctor : public FunctorBase
{
    typedef typename TrimTypeList<TYPELIST_6(
        A1, A2, A3, A4, A5, A6) >::Result Args;
    typedef FunctorInfo<R, Args> Info;
    typedef FunctorImpl<Info> Impl;

public:
    /**
     * Creates a null functor.
     */
    inline WvFunctor(int = 0) :
        FunctorBase() { }
       
    /**
     * Creates a copy of a functor.
     * @param other the functor to copy
     */
    inline WvFunctor(const WvFunctor& other) :
        FunctorBase(other) { }
    
    /**
     * Sets the functor to a copy of the other functor.
     * @param other the functor to copy
     * @return reference to self
     */
    inline WvFunctor& operator= (const WvFunctor& other)
    {
        set(other);
        return *this;
    }
    
    /**
     * Sets the functor to null.
     * @return reference to self
     */
    inline WvFunctor& operator= (int)
    {
        clear();
        return *this;
    }

    /**
     * Returns true if the functors are equal.
     * <p>
     * Considers the deep structure of both objects.  Functors may
     * be considered non-equal because their structures differ
     * due to their manner of construction such as the order by
     * which arguments may have been bound.
     * </p>
     * @param other the functor to compare against
     * @return true if equal
     */
    inline bool operator== (const WvFunctor& other)
    {
        return equals(other);
    }

    /**
     * Returns true if the functors are not equal.
     * <p>
     * Considers the deep structure of both objects.  Functors may
     * be considered non-equal because their structures differ
     * due to their manner of construction such as the order by
     * which arguments may have been bound.
     * </p>
     * @param other the functor to compare against
     * @return true if non-equal
     */
    inline bool operator!= (const WvFunctor& other)
    {
        return ! equals(other);
    }

    /*** Functor binding ***/
    
    /**
     * Creates a functor with 0 bound arguments.
     * @param fun the functor
     */
    template<class Fun>
    explicit WvFunctor(Fun fun) : FunctorBase(
        FunctorImplChooser<Info, Fun>::Result::create(fun)) { }

    /**
     * Creates a functor with 1 bound argument.
     * @param fun the functor
     * @param b1 the value to bind to the first argument
     */
    template<class B1, class Fun>
    WvFunctor(Fun fun, B1 b1) :
        FunctorBase(bind1st<B1>(
            FunctorImplChooser<
                FunctorInfo<R, TypeList<B1, Args> >,
                Fun>::Result::create(fun),
            b1)) { }

    /**
     * Creates a functor with 2 bound arguments.
     * @param fun the functor
     * @param b1 the value to bind to the first argument
     * @param b2 the value to bind to the second argument
     */
    template<class B1, class B2, class Fun>
    WvFunctor(Fun fun, B1 b1, B2 b2) :
        FunctorBase(bind1st<B2>(bind1st<B1>(
            FunctorImplChooser<
                FunctorInfo<R, TypeList<B1, TypeList<B2, Args> > >,
                Fun>::Result::create(fun),
            b1), b2)) { }
    
    /**
     * Creates a functor with 3 bound arguments.
     * @param fun the functor
     * @param b1 the value to bind to the first argument
     * @param b2 the value to bind to the second argument
     * @param b3 the value to bind to the third argument
     */
    template<class B1, class B2, class B3, class Fun>
    WvFunctor(Fun fun, B1 b1, B2 b2, B3 b3) :
        FunctorBase(bind1st<B3>(bind1st<B2>(bind1st<B1>(
            FunctorImplChooser<
                FunctorInfo<R, TypeList<B1, TypeList<B2,
                    TypeList<B3, Args> > > >,
                Fun>::Result::create(fun),
            b1), b2), b3)) { }
            
    /**
     * Creates a functor with 4 bound arguments.
     * @param fun the functor
     * @param b1 the value to bind to the first argument
     * @param b2 the value to bind to the second argument
     * @param b3 the value to bind to the third argument
     * @param b4 the value to bind to the fourth argument
     */
    template<class B1, class B2, class B3, class B4, class Fun>
    WvFunctor(Fun fun, B1 b1, B2 b2, B3 b3, B4 b4) :
        FunctorBase(bind1st<B4>(bind1st<B3>(bind1st<B2>(
            bind1st<B1>(
            FunctorImplChooser<
                FunctorInfo<R, TypeList<B1, TypeList<B2,
                    TypeList<B3, TypeList<B4, Args> > > > >,
                Fun>::Result::create(fun),
            b1), b2), b3), b4)) { }

    /**
     * Creates a functor with 5 bound arguments.
     * @param fun the functor
     * @param b1 the value to bind to the first argument
     * @param b2 the value to bind to the second argument
     * @param b3 the value to bind to the third argument
     * @param b4 the value to bind to the fourth argument
     * @param b5 the value to bind to the fifth argument
     */
    template<class B1, class B2, class B3, class B4, class B5,
        class Fun>
    WvFunctor(Fun fun, B1 b1, B2 b2, B3 b3, B4 b4, B5 b5) :
        FunctorBase(bind1st<B5>(bind1st<B4>(bind1st<B3>(
            bind1st<B2>(bind1st<B1>(
            FunctorImplChooser<
                FunctorInfo<R, TypeList<B1, TypeList<B2,
                    TypeList<B3, TypeList<B4, TypeList<B5,
                    Args> > > > > >,
                Fun>::Result::create(fun),
            b1), b2), b3), b4), b5)) { }
            
    /**
     * Creates a functor with 6 bound arguments.
     * @param fun the functor
     * @param b1 the value to bind to the first argument
     * @param b2 the value to bind to the second argument
     * @param b3 the value to bind to the third argument
     * @param b4 the value to bind to the fourth argument
     * @param b5 the value to bind to the fifth argument
     * @param b6 the value to bind to the sixth argument
     */
    template<class B1, class B2, class B3, class B4, class B5,
        class B6, class Fun>
    WvFunctor(Fun fun, B1 b1, B2 b2, B3 b3, B4 b4, B5 b5, B6 b6) :
        FunctorBase(bind1st<B6>(bind1st<B5>(bind1st<B4>(
            bind1st<B3>(bind1st<B2>(bind1st<B1>(
            FunctorImplChooser<
                FunctorInfo<R, TypeList<B1, TypeList<B2,
                    TypeList<B3, TypeList<B4, TypeList<B5,
                    TypeList<B6, Args> > > > > > >,
                Fun>::Result::create(fun),
            b1), b2), b3), b4), b5), b6)) { }

    /*** Invocation ***/
    
    inline R operator()() const
    {
        return (*static_cast<Impl*>(impl))();
    }
    inline R operator()(A1 a1) const
    {
        return (*static_cast<Impl*>(impl))(a1);
    }
    inline R operator()(A1 a1, A2 a2) const
    {
        return (*static_cast<Impl*>(impl))(a1, a2);
    }
    inline R operator()(A1 a1, A2 a2, A3 a3) const
    {
        return (*static_cast<Impl*>(impl))(a1, a2, a3);
    }
    inline R operator()(A1 a1, A2 a2, A3 a3, A4 a4) const
    {
        return (*static_cast<Impl*>(impl))(a1, a2, a3, a4);
    }
    inline R operator()(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) const
    {
        return (*static_cast<Impl*>(impl))(a1, a2, a3, a4, a5);
    }
    inline R operator()(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6) const
    {
        return (*static_cast<Impl*>(impl))(a1, a2, a3, a4, a5, a6);
    }
};

}; // namespace

/**
 * Export WvFunctor by default.
 */
using WvGeneric::WvFunctor;

#endif // __WVFUNCTOR_H
