/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 * 
 * A UniConf generator that stores keys in memory.
 */
#include "uniconftemp.h"
#include "wvmoniker.h"
#include "wvlog.h"

static UniConfGen *creator(WvStringParm, IObject *, void *)
{
    return new UniConfTempGen();
}

static WvMoniker<UniConfGen> reg("temp", creator);

/***** UniConfTempGen *****/

UniConfTempGen::UniConfTempGen()
    : root(NULL)
{
}


UniConfTempGen::~UniConfTempGen()
{
    delete root;
}


WvString UniConfTempGen::get(const UniConfKey &key)
{
    if (root)
    {
        UniConfValueTree *node = root->find(key);
        if (node)
            return node->value();
    }
    return WvString::null;
}


bool UniConfTempGen::set(const UniConfKey &key, WvStringParm value)
{
    if (value.isnull())
    {
        // remove a subtree
        if (root)
        {
            UniConfValueTree *node = root->find(key);
            if (node)
            {
                delete node;
                if (node == root)
                    root = NULL;
                dirty = true;
                delta(key, UniConfDepth::INFINITE);
            }
        }
    }
    else
    {
        UniConfValueTree *node = root;
        UniConfValueTree *prev = NULL;
        UniConfKey prevkey;
	
        UniConfKey::Iter it(key);
        it.rewind();
        for (;;)
        {
            bool more = it.next(); // not the last node in the key?
	    
            if (!node)
            {
		// we'll have to create the sub-node, since we couldn't
		// find the most recent part of the key.
                node = new UniConfValueTree(prev, prevkey,
					    more ? WvString("") : value);
                dirty = true;
                if (!prev) // we just created the root
                    root = node;
                delta(node->fullkey(), UniConfDepth::ONE);
                if (!more)
                    break; // done!
            }
            else if (!more)
            {
		// don't have to create the most recent sub-node, but there
		// are no more sub-nodes; that means we're changing the value
		// of an existing node.
                if (value != node->value())
                {
                    node->setvalue(value);
                    dirty = true;
                    delta(node->fullkey(), UniConfDepth::ONE);
                }
                break;
            }
            prevkey = *it;
            prev = node;
            node = prev->findchild(prevkey);
        }
	
	assert(node);
    }
    
    return true;
}


bool UniConfTempGen::zap(const UniConfKey &key)
{
    UniConfValueTree *oldroot = root;
    if (oldroot)
    {
        root = NULL;
        dirty = true;
        delta(UniConfKey::EMPTY, UniConfDepth::INFINITE);
        delete oldroot;
    }
    return true;
}


bool UniConfTempGen::exists(const UniConfKey &key)
{
    if (root)
    {
        UniConfValueTree *node = root->find(key);
        return node != NULL;
    }
    return false;
}


bool UniConfTempGen::haschildren(const UniConfKey &key)
{
    if (root)
    {
        UniConfValueTree *node = root->find(key);
        return node != NULL && node->haschildren();
    }
    return false;
}


UniConfGen::Iter *UniConfTempGen::iterator(const UniConfKey &key)
{
    if (root)
    {
        UniConfValueTree *node = root->find(key);
        if (node)
            return new NodeIter(this, UniConfValueTree::Iter(*node));
    }
    return new NullIter();
}



/***** UniConfTempGen::NodeIter *****/

UniConfTempGen::NodeIter::NodeIter(UniConfTempGen *gen,
    const UniConfValueTree::Iter &it) :
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
