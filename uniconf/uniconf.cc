/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * UniConf is the new, improved, hierarchical version of WvConf.  It stores
 * strings in a hierarchy and can load/save them from "various places."
 * 
 * See uniconf.h.
 */
#include "uniconf.h"
#include "uniconfnull.h"
#include "wvstream.h"
#include "wvstringtable.h"
#include "uniconfiter.h"
#include <assert.h>

/***** UniConf *****/

UniConf::UniConf() :
    Tree(NULL, UniConfKey::EMPTY),
    prefix(UniConfKey::EMPTY), uniconfroot(NULL), defaults(NULL)
{
    uniconfroot = new UniConfRoot();
}


UniConf::UniConf(UniConf *parent, const UniConfKey &name) :
    Tree(parent, name),
    prefix(UniConfKey(parent->prefix, name)),
    uniconfroot(parent->uniconfroot), defaults(NULL)
{
}


UniConf::~UniConf()
{
    if (prefix.isempty())
        delete uniconfroot;
}


bool UniConf::haschildren()
{
    return uniconfroot->haschildren(prefix);
}


WvString UniConf::get(const UniConfKey &key)
{
    return uniconfroot->get(UniConfKey(prefix, key));
}


bool UniConf::set(const UniConfKey &key, WvStringParm value)
{
    return uniconfroot->set(UniConfKey(prefix, key), value);
}


bool UniConf::zap(const UniConfKey &key)
{
    return uniconfroot->zap(UniConfKey(prefix, key));
}


bool UniConf::commit(const UniConfKey &key, UniConf::Depth depth)
{
    return uniconfroot->commit(UniConfKey(prefix, key), depth);
}


bool UniConf::refresh(const UniConfKey &key, UniConf::Depth depth)
{
    return uniconfroot->refresh(UniConfKey(prefix, key), depth);
}


UniConfGen *UniConf::mount(const UniConfLocation &location)
{
    return uniconfroot->mount(prefix, location, true);
}


void UniConf::mountgen(UniConfGen *gen)
{
    uniconfroot->mountgen(prefix, gen, true);
}


void UniConf::unmount()
{
    uniconfroot->unmount(prefix);
}


bool UniConf::isok()
{
    return uniconfroot->isok(prefix);
}


bool UniConf::ismountpoint()
{
    return uniconfroot->ismountpoint(prefix);
}


void UniConf::dump(WvStream &stream, bool everything)
{
    UniConf::RecursiveIter it(*this);
    for (it.rewind(); it.next(); )
    {
        if (everything || !! it->value())
            stream.print("%s = %s\n", it->fullkey(), it->value());
    }
}



/***** UniConf::Iter *****/

UniConf::Iter::Iter(UniConf &root) :
    UniConfRoot::Iter(*root.uniconfroot, root.prefix),
    xroot(& root)
{
}


UniConf *UniConf::Iter::ptr() const
{
    return xroot->findormake(key());
}



/***** UniConf::RecursiveIter *****/

UniConf::RecursiveIter::RecursiveIter(UniConf &_root,
    UniConf::Depth _depth) :
    top(_root), depth(_depth)
{
}


void UniConf::RecursiveIter::rewind()
{
    itlist.zap();
    first = false;
    switch (depth)
    {
        case UniConf::ZERO:
            first = true;
            break;

        case UniConf::ONE:
        case UniConf::INFINITE:
            first = true;
            // fall through

        case UniConf::CHILDREN:
        case UniConf::DESCENDENTS:
            itlist.append(& top, false);
            top.rewind();
            break;
    }
}


bool UniConf::RecursiveIter::next()
{
    if (first)
    {
        first = false;
        current = root();
        return true;
    }

    IterList::Iter itlistit(itlist);
    for (itlistit.rewind(); itlistit.next(); )
    {
        UniConf::Iter &it = itlistit();
        if (it.next())
        {
            current = it.ptr();
            if ((depth == UniConf::INFINITE ||
                depth == UniConf::DESCENDENTS) &&
                current->haschildren())
            {
                UniConf::Iter *subit = new UniConf::Iter(*current);
                subit->rewind();
                itlist.prepend(subit, true);
            }
            return true;
        }
        itlistit.xunlink();
    }
    return false;
}



#if 0
static int find_wildcard_depth(const UniConfKey &k)
{
    int depth = 0;
    int segments = k.numsegments();
    while (depth < segments)
    {
        if (k.segment(depth) == UniConfKey::ANY)
            break;
	depth++;
    }
    return depth;
}
    

// I hate constructors.
UniConf::XIter::XIter(UniConf &_top, const UniConfKey &_key) :
    skiplevel(find_wildcard_depth(_key)),
    top(_top.find(_key.first(skiplevel))),
    key(_key.removefirst(skiplevel)),
    _toplink(top, false),
    toplink(top ? &_toplink : NULL),
    i((top && top->check_children()) ?
        *top->children : null_wvhconfdict)
{
    subiter = NULL; 
}


WvLink *UniConf::XIter::_next()
{
    if (key.isempty()) // we're the innermost object
    {
	if (++going == 1)
	    return toplink;
	else
	    return NULL;
    }
    
    do
    {
	if (!subiter && i.ptr())
	{
	    subiter = new XIter(*i, key.removefirst(1));
	    subiter->rewind();
	}
	
	if (subiter)
	{
	    WvLink *l = subiter->next();
	    if (l) return l;
	    unsub();
	}
    } while (i.next());
    
    return NULL;
}
#endif



/***** UniConfInfoTree *****/

UniConfInfoTree::UniConfInfoTree(UniConfInfoTree *parent,
    const UniConfKey &key) :
    UniConfTree<UniConfInfoTree>(parent, key), generator(NULL)
{
}


UniConfInfoTree::~UniConfInfoTree()
{
}


UniConfInfoTree *UniConfInfoTree::findnearest(const UniConfKey &key,
    int &split)
{
    split = 0;
    UniConfInfoTree *node = this;
    UniConfKey::Iter it(key);
    for (it.rewind(); it.next(); ++split)
    {
        UniConfInfoTree *next = node->findchild(it());
        if (! next)
            break;
        node = next;
    }
    return node;
}


UniConfInfoTree *UniConfInfoTree::findormake(const UniConfKey &key)
{
    UniConfInfoTree *node = this;
    UniConfKey::Iter it(key);
    for (it.rewind(); it.next(); )
    {
        UniConfInfoTree *prev = node;
        node = prev->findchild(it());
        if (! node)
            node = new UniConfInfoTree(prev, it());
    }
    return node;
}



/***** UniConfInfoTree::NodeIter *****/

UniConfInfoTree::NodeIter::NodeIter(UniConfInfoTree &root,
    const UniConfKey &key) :
    xkey(key)
{
    bestnode = root.findnearest(key, bestsplit);
}


void UniConfInfoTree::NodeIter::rewind()
{
    xnode = NULL;
}


bool UniConfInfoTree::NodeIter::next()
{
    if (! xnode)
    {
        xsplit = bestsplit;
        xnode = bestnode;
    }
    else if (xsplit != 0)
    {
        xsplit -= 1;
        xnode = xnode->parent();
    }
    else
        return false;
    return true;
}



/***** UniConfInfoTree::GenIter *****/

UniConfInfoTree::GenIter::GenIter(UniConfInfoTree &root,
    const UniConfKey &key) :
    UniConfInfoTree::NodeIter(root, key)
{
}


void UniConfInfoTree::GenIter::rewind()
{
    UniConfInfoTree::NodeIter::rewind();
}


bool UniConfInfoTree::GenIter::next()
{
    while (UniConfInfoTree::NodeIter::next())
    {
        if (node()->generator)
            return true;
    }
    return false;
}



/***** UniConfRoot *****/

UniConfRoot::UniConfRoot() :
    root(NULL, UniConfKey::EMPTY), streamlist(NULL)
{
}


UniConfRoot::~UniConfRoot()
{
}


WvString UniConfRoot::get(const UniConfKey &key, WvStringParm defvalue)
{
    UniConfInfoTree::GenIter it(root, key);
    for (it.rewind(); it.next(); )
    {
        UniConfGen *gen = it.ptr();
        WvString result = gen->get(it.tail());
        if (! result.isnull())
            return result;
    }
    return defvalue;
}


bool UniConfRoot::set(const UniConfKey &key, WvStringParm value)
{
    UniConfInfoTree::GenIter it(root, key);
    // locate the generator that defines the key, if any
    for (it.rewind(); it.next(); )
    {
        UniConfGen *gen = it.ptr();
        if (gen->exists(it.tail()))
            return gen->set(it.tail(), value);
    }
    
    // otherwise, set the key on the first generator that works
    for (it.rewind(); it.next(); )
    {
        UniConfGen *gen = it.ptr();
        if (gen->set(it.tail(), value))
            return true;
    }
    return false;
}


bool UniConfRoot::zap(const UniConfKey &key)
{
    bool success = true;
    UniConfInfoTree::GenIter it(root, key);
    for (it.rewind(); it.next(); )
    {
        UniConfGen *gen = it.ptr();
        if (! gen->zap(it.tail()))
            success = false;
    }
    return success;
}


bool UniConfRoot::exists(const UniConfKey &key)
{
    UniConfInfoTree *node = root.find(key);
    if (node)
        return true;
        
    UniConfInfoTree::GenIter it(root, key);
    for (it.rewind(); it.next(); )
    {
        UniConfGen *gen = it.ptr();
        if (gen->exists(it.tail()))
            return true;
    }
    return false;
}


bool UniConfRoot::haschildren(const UniConfKey &key)
{
    UniConfInfoTree *node = root.find(key);
    if (node && node->haschildren())
        return true;

    UniConfInfoTree::GenIter it(root, key);
    for (it.rewind(); it.next(); )
    {
        UniConfGen *gen = it.ptr();
        if (gen->haschildren(it.tail()))
            return true;
    }
    return false;
}


bool UniConfRoot::refresh(const UniConfKey &key, UniConf::Depth depth)
{
    // FIXME: need to handle recursion depth correctly
    bool success = true;
    UniConfInfoTree::GenIter it(root, key);
    for (it.rewind(); it.next(); )
    {
        UniConfGen *gen = it.ptr();
        if (! gen->refresh(it.tail(), UniConf::INFINITE))
            success = false;
    }
    return success;
}
 
 
bool UniConfRoot::commit(const UniConfKey &key, UniConf::Depth depth)
{
    // FIXME: need to handle recursion depth correctly
    bool success = true;
    UniConfInfoTree::GenIter it(root, key);
    for (it.rewind(); it.next(); )
    {
        UniConfGen *gen = it.ptr();
        if (! gen->commit(it.tail(), UniConf::INFINITE))
            success = false;
    }
    return success;
}


void UniConfRoot::attach(WvStreamList *_streamlist)
{
    assert(streamlist == NULL);
    assert(_streamlist != NULL);
    streamlist = _streamlist;
    // FIXME: must iterator over all existing generators
}


void UniConfRoot::detach(WvStreamList *_streamlist)
{
    assert(streamlist != NULL);
    assert(streamlist == _streamlist);
    streamlist = NULL;
    // FIXME: must iterator over all existing generators
}


UniConfGen *UniConfRoot::mount(const UniConfKey &key,
    const UniConfLocation &location, bool refresh)
{
    UniConfGen *gen = UniConfGenFactoryRegistry::instance()->
        newgen(location);
    mountgen(key, gen, refresh);
    return gen;
}


void UniConfRoot::mountgen(const UniConfKey &key,
    UniConfGen *gen, bool refresh)
{
    unmount(key);
    UniConfInfoTree *node = root.findormake(key);
    node->generator = gen;
    if (refresh)
        gen->refresh(UniConfKey::EMPTY, UniConf::INFINITE);
}


void UniConfRoot::unmount(const UniConfKey &key)
{
    UniConfInfoTree *node = root.find(key);
    if (node)
    {
        delete node->generator;
        node->generator = NULL;
    }
}


bool UniConfRoot::isok(const UniConfKey &key)
{
    UniConfInfoTree::GenIter it(root, key);
    for (it.rewind(); it.next(); )
    {
        UniConfGen *gen = it.ptr();
        return gen->isok();
    }
    return true;
}


bool UniConfRoot::ismountpoint(const UniConfKey &key)
{
    UniConfInfoTree *node = root.find(key);
    return node != NULL && node->generator;
}


void UniConfRoot::prune(UniConfInfoTree *node)
{
    while (node != & root && ! node->isessential())
    {
        UniConfInfoTree *next = node->parent();
        delete node;
        node = next;
    }
}



/***** UniConfRoot::Iter *****/

UniConfRoot::Iter::Iter(UniConfRoot &root, const UniConfKey &key) :
    xroot(& root), xkey(key), genit(root.root, key),
    hack(71), hackit(hack)
{
}


void UniConfRoot::Iter::rewind()
{
    hack.zap();

    // add mountpoint nodes
    UniConfInfoTree *node = xroot->root.find(xkey);
    if (node)
    {
        UniConfInfoTree::Iter nodeit(*node);
        for (nodeit.rewind(); nodeit.next(); )
        {
            hack.add(new WvString(nodeit->key()), true);
        }
    }

    // add key nodes
    for (genit.rewind(); genit.next(); )
    {
        UniConfGen *gen = genit.ptr();
        UniConfAbstractIter *keyit = gen->iterator(genit.tail());
        for (keyit->rewind(); keyit->next(); )
        {
            hack.add(new WvString(keyit->key()), true);
        }
        delete keyit;
    }

    hackit.rewind();
}


bool UniConfRoot::Iter::next()
{
    return hackit.next();
}


UniConfKey UniConfRoot::Iter::key() const
{
    return UniConfKey(hackit());
}
