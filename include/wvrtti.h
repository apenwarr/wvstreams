/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Run-Time Type Identification abstractions.
 * 
 * This module is intended to be used as a workaround for missing
 * compiler features when WvStreams is compiled without RTTI
 * support to reduce its memory and disk footprint.
 */
#ifndef __WVRTTI_H
#define __WVRTTI_H

/**
 * Provides run time type support.
 * Use the WVTYPE macro to attach one of these to a class.
 */
class WvType
{
protected:
    static int nextid;
    const WvType *parent;
    const int id;

public:
    // the void type
    static const WvType WVTYPE_VOID;

    /**
     * Creates a new run time type support object.
     * @param parent a pointer to the supertype, or NULL
     */
    explicit WvType(const WvType *_parent);

    /**
     * Returns true if the types are the same.
     */
    inline bool operator==(const WvType &other) const
    {
        return id == other.id;
    }
    
    /**
     * Returns true if the types are different.
     */
    inline bool operator!=(const WvType &other) const
    {
        return id != other.id;
    }

    /**
     * Returns true if this type is a subtype of the other, or if
     * they are both the same.
     * Every type is a subtype of void.
     */
    bool subtypeof(const WvType &other) const;
};


/**
 * Helper struct for the wvtypeof() function.
 */
template<class T>
struct WvTypeOfFunctor
{
    const WvType &operator()()
    {
        return T::WVTYPE_STATIC();
    }
};
struct WvTypeOfFunctor<void>
{
    const WvType &operator()()
    {
        return WvType::WVTYPE_VOID;
    }
};


/**
 * Returns a reference to the WvTypeBase of the specified class.
 */
template<class T>
inline const WvType &wvtypeof()
{
    return WvTypeOfFunctor<T>()();
}


/**
 * Attaches type information to a class.
 * Insert WVTYPE(S) at the beginning of the class declaration
 * where S is the type of the superclass or 'void'.
 *
 * This is actually a lot trickier than it looks because we want
 * to avoid having to initialize the WvType instance separately
 * since this would make it even more tedious to graft type
 * information onto classes due to the duplication of effort.
 * C++ forbid in-class initialization of non-integer
 * static members, so instead we take recourse to static locals
 * which have no such restriction.
 */
#define WVTYPE(S) \
public: \
    static const WvType &WVTYPE_STATIC() \
    { \
        static const WvType instance(& wvtypeof<S>()); \
        return instance; \
    } \
    virtual const WvType &WVTYPE_INSTANCE() const \
    { \
        return WVTYPE_STATIC(); \
    } \
private:


/**
 * Helper struct for the wvinstanceof() function.
 */
template<class S>
struct WvInstanceOfHelper;

template<class S>
struct WvInstanceOfHelper<S *>
{
    template<class T>
    inline bool operator()(const T *obj)
    {
        return obj && obj->WVTYPE_INSTANCE().subtypeof(wvtypeof<S>());
    }
};

template<class S>
struct WvInstanceOfHelper<const S *>
{
    template<class T>
    inline bool operator()(const T *obj)
    {
        return obj->WVTYPE_INSTANCE().subtypeof(wvtypeof<S>());
    }
};


/**
 * Returns true if the object is an instance of the specified type.
 */
template<class S, class T>
inline bool wvinstanceof(const T *obj)
{
    return obj && WvInstanceOfHelper<S>()(obj);
}


/**
 * Replaces dynamic_cast.
 */
template<class S, class T>
inline S wvdynamic_cast(T obj)
{
    return wvinstanceof<S>(obj) ? static_cast<S>(obj) : (S)0;
}

#endif // __WVRTTI_H
