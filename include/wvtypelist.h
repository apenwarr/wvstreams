/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 *
 * Declares typelists for use with generic programming techniques.
 */
#ifndef __WVTYPELIST_H
#define __WVTYPELIST_H

namespace WvGeneric
{

/**
 * An undefined type, used as a marker in type lists.
 */
class NullType;


/**
 * The TypeList template allows the creation of arbitarily long lists
 * of types that can be inspected and manipulated in a simple generic
 * manner.  
 */
template<class H, class T>
struct TypeList
{
    typedef H Head;
    typedef T Tail;
};

#define TYPELIST_0 \
    NullType
#define TYPELIST_X(_args_...) \
    NullType
#define TYPELIST_1(T1) \
    TypeList<T1, TYPELIST_0 >
#define TYPELIST_2(T1, T2) \
    TypeList<T1, TYPELIST_1(T2) >
#define TYPELIST_3(T1, T2, T3) \
    TypeList<T1, TYPELIST_2(T2, T3) >
#define TYPELIST_4(T1, T2, T3, T4) \
    TypeList<T1, TYPELIST_3(T2, T3, T4) >
#define TYPELIST_5(T1, T2, T3, T4, T5) \
    TypeList<T1, TYPELIST_4(T2, T3, T4, T5) >
#define TYPELIST_6(T1, T2, T3, T4, T5, T6) \
    TypeList<T1, TYPELIST_5(T2, T3, T4, T5, T6) >


/**
 * The TypeAt template takes a type list and a zero-based index and
 * returns via the Result typedef the indexed type in the list, or
 * NullType if no such element is found.
 */
template<class TL, unsigned int index>
struct TypeAt;

template<unsigned int index>
struct TypeAt<NullType, index>
{
    typedef NullType Result;
};

template<class Head, class Tail>
struct TypeAt<TypeList<Head, Tail>, 0>
{
    typedef Head Result;
};

template<class Head, class Tail, unsigned int index>
struct TypeAt<TypeList<Head, Tail>, index>
{
    typedef typename TypeAt<Tail, index - 1>::Result Result;
};


/**
 * The TrimTypeList template takes a type list and returns via the
 * Result typedef a new type list truncated at the first occurrence
 * of NullType.
 */
template<class TL>
struct TrimTypeList;

template<>
struct TrimTypeList<NullType>
{
    typedef NullType Result;
};

template<class Tail>
struct TrimTypeList<TypeList<NullType, Tail> >
{
    typedef NullType Result;
};

template<class Head, class Tail>
struct TrimTypeList<TypeList<Head, Tail> >
{
    typedef TypeList<Head, typename TrimTypeList<Tail>::Result > Result;
};


}; // namespace

#endif // __WVTYPELIST_H
