/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 *
 * Declares a smart bound functor type.
 */
#include "wvfunctor.h"

namespace WvGeneric
{

/***** FunctorImplBase *****/

#ifdef NO_RTTI
int FunctorImplBase::PREVTYPE = 0;
#endif

FunctorImplBase::~FunctorImplBase()
{
}


/***** FunctorBase *****/

FunctorBase::FunctorBase(const FunctorBase &other) :
    impl(other.impl ? other.impl->clone() : 0)
{
}


FunctorBase::~FunctorBase()
{
    delete impl;
}


void FunctorBase::set(const FunctorBase& other)
{
    delete impl;
    impl = other.impl ? other.impl->clone() : 0;
}


void FunctorBase::clear()
{
    delete impl;
    impl = 0;
}


bool FunctorBase::equals(const FunctorBase& other) const
{
    // check for null
    if (! impl)
        return ! other.impl;
    if (! other.impl)
        return false;

    // do deep comparison
    // there is no point doing a shallow comparison because
    // functor implementations are always cloned
    return impl->equals(*other.impl);
}


};
