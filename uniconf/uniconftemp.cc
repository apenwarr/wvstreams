/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 */
 
/** /file
 * A UniConf generator that stores keys in memory.
 */
#include "uniconftemp.h"

/***** UniConfTempGen *****/

UniConfTempGen::UniConfTempGen() :
    root(NULL, UniConfKey::EMPTY, WvString::null)
{
}


UniConfTempGen::~UniConfTempGen()
{
}


UniConfLocation UniConfTempGen::location() const
{
    return UniConfLocation("temp://");
}


WvString UniConfTempGen::get(const UniConfKey &key)
{
    UniConfTree *node = root.find(key);
    if (node)
        return node->value();
    return WvString::null;
}


bool UniConfTempGen::set(const UniConfKey &key, WvStringParm value)
{
    if (value.isnull())
    {
        if (key.isempty())
        {
            // special case removing the root
            if (root.haschildren())
            {
                root.zap();
                dirty = true;
            }
            if (! root.value().isnull())
            {
                root.setvalue(WvString::null);
                dirty = true;
            }
        }
        else
        {
            // remove a non-root subtree
            UniConfTree *node = root.find(key);
            if (node)
            {
                delete node;
                dirty = true;
            }
        }
    }
    else
    {
        UniConfTree *node = & root;
        UniConfKey::Iter it(key);
        it.rewind();
        while (it.next())
        {
            UniConfTree *prev = node;
            node = node->findchild(it());
            if (! node)
            {
                node = new UniConfTree(prev, it(), "");
                dirty = true;
            }
        }
        if (value != node->value())
        {
            node->setvalue(value);
            dirty = true;
        }
    }
    return true;
}


bool UniConfTempGen::zap(const UniConfKey &key)
{
    UniConfTree *node = root.find(key);
    if (node && node->haschildren())
    {
        node->zap();
        dirty = true;
    }
    return true;
}


bool UniConfTempGen::exists(const UniConfKey &key)
{
    UniConfTree *node = root.find(key);
    return node != NULL;
}


bool UniConfTempGen::haschildren(const UniConfKey &key)
{
    UniConfTree *node = root.find(key);
    return node != NULL && node->haschildren();
}


UniConfGen::Iter *UniConfTempGen::iterator(
    const UniConfKey &key)
{
    UniConfTree *node = root.find(key);
    if (node)
        return new NodeIter(this, UniConfTree::Iter(*node));
    return new NullIter();
}



/***** UniConfTempGen::NodeIter *****/

UniConfTempGen::NodeIter::NodeIter(UniConfTempGen *gen,
    const UniConfTree::Iter &it) :
    xgen(gen), xit(it)
{
}


UniConfTempGen::NodeIter::NodeIter(
    const UniConfTempGen::NodeIter &other) :
    UniConfAbstractIter(other),
    xgen(other.xgen), xit(other.xit)
{
}


UniConfTempGen::NodeIter::~NodeIter()
{
}


UniConfTempGen::NodeIter *UniConfTempGen::NodeIter::clone() const
{
    return new UniConfTempGen::NodeIter(*this);
}


void UniConfTempGen::NodeIter::rewind()
{
    xit.rewind();
}


bool UniConfTempGen::NodeIter::next()
{
    return xit.next();
}


UniConfKey UniConfTempGen::NodeIter::key() const
{
    return xit->key();
}



/***** UniConfTempGenFactory *****/

UniConfTempGen *UniConfTempGenFactory::newgen(
    const UniConfLocation &location)
{
    return new UniConfTempGen();
}
