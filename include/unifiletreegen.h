/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * A generator for .ini files.
 */
#ifndef __UNICONFFILETREEGEN_H
#define __UNICONFFILETREEGEN_H

#include "uniconfgen.h"
#include "unitempgen.h"
#include "wvlog.h"
#include "unimounttreegen.h"

class UniConfFileTreeGen : public UniMountTreeGen//UniConfGen
{
public:
    WvString basedir, moniker;
    WvLog log;
//    UniConfValueTree root;
    
    UniConfFileTreeGen(WvStringParm _basedir, WvStringParm _moniker);
    virtual ~UniConfFileTreeGen() { }

    /***** Overridden members *****/

    virtual bool refresh();
/*    virtual WvString get(const UniConfKey &key);
    virtual bool exists(const UniConfKey &key);
    virtual bool haschildren(const UniConfKey &key);
    virtual void set(const UniConfKey &key, WvStringParm) { }

    virtual Iter *iterator(const UniConfKey &key);
*/
private:
//    UniConfValueTree *maketree(const UniConfKey &key);
    class NodeIter;
};


class UniConfFileTreeGen::NodeIter : public UniConfFileTreeGen::Iter
{
protected:
    UniConfValueTree::Iter xit;

public:
    NodeIter(UniConfValueTree &node) : xit(node)
        { }

    /***** Overridden methods *****/

    virtual void rewind()
        { xit.rewind(); }
    virtual bool next()
        { return xit.next(); }
    virtual UniConfKey key() const
        { return xit->key(); }
};

#endif // __UNICONFFILETREEGEN_H
