/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Run-Time Type Identification abstractions.
 */
#include "wvrtti.h"

/***** WvType *****/

// the next type id to assign
int WvType::nextid = 0;

// the type for void
const WvType WvType::WVTYPE_VOID(0);


WvType::WvType(const WvType *_parent) :
    parent(_parent), id(nextid++)
{
}


bool WvType::subtypeof(const WvType &other) const
{
    if (*this == other)
        return true;
        
    for (const WvType *type = parent; type; type = type->parent)
        if (*type == other)
            return true;
    return false;
}
