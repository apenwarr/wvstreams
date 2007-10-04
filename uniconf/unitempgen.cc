/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002-2005 Net Integration Technologies, Inc.
 * 
 * A UniConf generator that stores keys in memory.
 */
#include "unitempgen.h"
#include "wvmoniker.h"
#include "wvlog.h"
#include "wvstringcache.h"
#include "unilistiter.h"

static IUniConfGen *creator(WvStringParm, IObject*)
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

void UniTempGen::notify_deleted(const UniConfValueTree *node, void *)
{
    delta(node->fullkey(), WvString::null);
}

void UniTempGen::set(const UniConfKey &_key, WvStringParm _value)
{
    WvString value(scache.get(_value));
    
    hold_delta();
    UniConfKey key = _key;
    // FIXME: Use key.hastrailingslash(), it's shorter and easier and faster
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
                hold_delta();
                // Issue notifications for every key that gets deleted.
                node->visit(wv::bind(&UniTempGen::notify_deleted, this,
				     wv::_1, wv::_2),
			    NULL, false, true);
                delete node;
                if (node == root)
                    root = NULL;
                dirty = true;
                unhold_delta();
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
					    more ? WvString::empty : value);
                dirty = true;
                if (!prev) // we just created the root
                    root = node;
		if (more)
		    delta(node->fullkey(), WvString::empty); // AUTO-VIVIFIED
		else
		{
		    delta(node->fullkey(), value); // ADDED
		    break; // done!
		}
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


void UniTempGen::setv(const UniConfPairList &pairs)
{
    setv_naive(pairs);
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
		it->add(i->key(), i->value());
            return it;
	}
    }
    return NULL;
}


void UniTempGen::commit()
{
    UniConfGen::commit();
}


bool UniTempGen::refresh()
{
    return UniConfGen::refresh();
}
