/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * UniConfDefGen is a UniConfGen for retrieving data from the
 * UniConfDaemon with defaults.
 */

#ifndef __UNICONFLISTGEN_H
#define __UNICONFLISTGEN_H

#include "uniconfgen.h"

class UniConfListGen : public UniConfGen
{
public:
    UniConfListGen(UniConfGenList *_l) : l(_l), i(*_l) { }
    virtual UniConfListGen::~UniConfListGen() { delete l; }

    UniConfGenList *l;
    UniConfGenList::Iter i;

    /***** Overridden members *****/

    virtual bool commit(const UniConfKey &key, UniConfDepth::Type depth); 
    virtual bool refresh(const UniConfKey &key, UniConfDepth::Type depth);
    virtual WvString get(const UniConfKey &key);
    virtual bool set(const UniConfKey &key, WvStringParm value);
    virtual bool zap(const UniConfKey &key);
    virtual bool exists(const UniConfKey &key);
    virtual bool haschildren(const UniConfKey &key);
    virtual bool isok();
    virtual Iter *iterator(const UniConfKey &key);

//    class IterIter;
};



/**
 * An iterator over UniConf iterators.
 */
/*class UniConfListGen::IterIter : public UniConfListGen::Iter
{
protected:
    
public:


}; */



#endif // __UNICONFLISTGEN_H
