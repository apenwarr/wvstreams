/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 * 
 * A UniConf generator that stores keys in memory.
 */
#include "unitempgen.h"
#include "wvmoniker.h"
#include "wvlog.h"

static IUniConfGen *creator(WvStringParm, IObject *, void *)
{
    return new UniTempGen();
}

static WvMoniker<IUniConfGen> reg("temp", creator);

/***** UniTempGen *****/

UniTempGen::UniTempGen()
    : root(NULL)
{
}


UniTempGen::~UniTempGen()
{
    delete root;
}


WvString UniTempGen::get(const UniConfKey &key)
{
    if (root)
    {
	// Look for an empty section at the end.
	if (!key.isempty() && key.last().isempty())
	    return WvString::null;
	UniConfValueTree *node = root->find(key);
        if (node)
            return node->value();
    }
    return WvString::null;
}


void UniTempGen::set(const UniConfKey &_key, WvStringParm value)
{
    hold_delta();
    UniConfKey key = _key;
    bool trailing_slash = false;
    if (!key.isempty())
    {
	// Look for an empty section at the end.
	UniConfKey last = key;
	key = last.pop(last.numsegments() - 1);
	if (last.isempty())
	    trailing_slash = true;
	else
	    key = _key;
    }

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
                delta(key, WvString::null); // REMOVED
            }
        }
    }
    else if (!trailing_slash)
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
					    more ? WvStringParm("") : value);
                dirty = true;
                if (!prev) // we just created the root
                    root = node;
                delta(node->fullkey(), value); // ADDED
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
                    delta(node->fullkey(), value); // CHANGED
                }
                break;
            }
            prevkey = *it;
            prev = node;
            node = prev->findchild(prevkey);
        }
	assert(node);
    }
    
    unhold_delta();
}


bool UniTempGen::exists(const UniConfKey &key)
{
    if (root)
    {
        UniConfValueTree *node = root->find(key);
        return node != NULL;
    }
    return false;
}


bool UniTempGen::haschildren(const UniConfKey &key)
{
    if (root)
    {
        UniConfValueTree *node = root->find(key);
        return node != NULL && node->haschildren();
    }
    return false;
}


UniConfGen::Iter *UniTempGen::iterator(const UniConfKey &key)
{
    if (root)
    {
        UniConfValueTree *node = root->find(key);
        if (node)
	{
	    ListIter *it = new ListIter(this);
	    UniConfValueTree::Iter i(*node);
	    for (i.rewind(); i.next(); )
	    {
		it->keys.append(new WvString(i->key()), true);
		it->values.append(new WvString(i->value()), true);
	    }
            return it;
	}
    }
    return NULL;
}

