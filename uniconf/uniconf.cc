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
    generator(NULL), defaults(NULL)
{
}


UniConf::UniConf(UniConf *parent, const UniConfKey &name) :
    Tree(parent, name),
    generator(NULL), defaults(NULL)
{
}


UniConf::~UniConf()
{
    if (generator)
	delete generator;
}


bool UniConf::haschildren()
{
    UniConfKey genkey(UniConfKey::EMPTY);
    UniConfGen *gen = findgen(genkey);
    if (gen)
        return gen->haschildren(genkey);
    return false;
}


WvString UniConf::get(const UniConfKey &key)
{
    UniConfKey genkey(key);
    UniConfGen *gen = findgen(genkey);
    if (gen)
        return gen->get(genkey);
    return WvString::null;
}


bool UniConf::set(const UniConfKey &key, WvStringParm value)
{
    UniConfKey genkey(key);
    UniConfGen *gen = findgen(genkey);
    if (gen)
        return gen->set(genkey, value);
    return false;
}


bool UniConf::zap(const UniConfKey &key)
{
    UniConf *node = find(key);
    if (node)
        node->zap_recursive();
    return true;
}


bool UniConf::zap_recursive()
{
    UniConf::Tree::Iter it(*this);
    for (it.rewind(); it.next(); )
    {
        UniConf *node = it.ptr();
        if (! node->zap_recursive())
            delete node;
    }
    return generator != NULL;
}


bool UniConf::commit(const UniConfKey &key, UniConf::Depth depth)
{
    UniConfKey genkey(key);
    UniConfGen *gen = findgen(genkey);
    bool success;
    if (gen)
        success = gen->commit(genkey, depth);
    else
        success = true;

    // AWFUL HACK
    UniConf::Tree::Iter it(*this);
    switch (depth)
    {
        case UniConf::ZERO:
            break;
            
        case UniConf::ONE:
        case UniConf::CHILDREN:
            for (it.rewind(); it.next(); )
                success = it->commit(key.removefirst(),
                    UniConf::ZERO) && success;
            break;
            
        case UniConf::INFINITE:
        case UniConf::DESCENDENTS:
            for (it.rewind(); it.next(); )
                success = it->commit(key.removefirst(),
                    UniConf::DESCENDENTS) && success;
            break;
    }
    return success;
}


bool UniConf::refresh(const UniConfKey &key, UniConf::Depth depth)
{
    UniConfKey genkey(key);
    UniConfGen *gen = findgen(genkey);
    bool success;
    if (gen)
        success = gen->refresh(genkey, depth);
    else
        success = true;
        
    // AWFUL HACK
    UniConf::Tree::Iter it(*this);
    switch (depth)
    {
        case UniConf::ZERO:
            break;
            
        case UniConf::ONE:
        case UniConf::CHILDREN:
            for (it.rewind(); it.next(); )
                success = it->refresh(key.removefirst(),
                    UniConf::ZERO) && success;
            break;
            
        case UniConf::INFINITE:
        case UniConf::DESCENDENTS:
            for (it.rewind(); it.next(); )
                success = it->refresh(key.removefirst(),
                    UniConf::DESCENDENTS) && success;
            break;
    }
    return success;
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

UniConfGen *UniConf::mount(const UniConfLocation &location)
{
    return mountgen(UniConfGenFactoryRegistry::instance()->
        newgen(location));
}


UniConfGen *UniConf::mountgen(UniConfGen *gen)
{
    unmount();
    generator = gen;
    return gen;
}


void UniConf::unmount()
{
    if (generator)
    {
        delete generator;
        generator = NULL;
    }
}


bool UniConf::isok()
{
    UniConf *node = genroot();
    if (node->generator)
        return node->generator->isok();
    return false;
}


/***** UniConf::Iter *****/

UniConf::Iter::Iter(UniConf &root) :
    xroot(& root), it(root), genit(NULL), hack(13)
{
}


UniConf::Iter::~Iter()
{
    delete genit;
}


void UniConf::Iter::rewind()
{
    delete genit;
    genit = NULL;
    UniConfKey genkey(UniConfKey::EMPTY);
    UniConfGen *gen = xroot->findgen(genkey);
    if (gen)
    {
        genit = gen->iterator(genkey);
        genit->rewind();
    }
    
    it.rewind();
    hack.zap();
    current = NULL;
}


bool UniConf::Iter::next()
{
    if (genit)
    {
        if (genit->next())
        {
            hack.add(new WvString(genit->key()), true);
            current = xroot->findormake(genit->key());
            return true;
        }
        delete genit;
        genit = NULL;
    }

    // look at other mounted subtrees
    while (it.next())
    {
        if (! hack[it->key()])
        {
            current = it.ptr();
            return true;
        }
    }
    return false;
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
