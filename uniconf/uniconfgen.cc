/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 * 
 * An abstract data container that backs a UniConf tree.
 */
#include "uniconfgen.h"
#include "strutils.h"

// FIXME: interfaces (IUniConfGen) shouldn't have implementations!
IUniConfGen::~IUniConfGen()
{
}

UUID_MAP_BEGIN(UniConfGen)
  UUID_MAP_ENTRY(IObject)
  UUID_MAP_ENTRY(IUniConfGen)
  UUID_MAP_END

UniConfGen::UniConfGen()
{
    hold_nesting = 0;
}


UniConfGen::~UniConfGen()
{
    assert(cblist.isempty());
}


void UniConfGen::hold_delta()
{
    hold_nesting++;
}


void UniConfGen::unhold_delta()
{
    assert(hold_nesting > 0);
    if (hold_nesting == 1)
        flush_delta();
    hold_nesting--;
}


void UniConfGen::clear_delta()
{
    deltas.zap();
}


void UniConfGen::flush_delta()
{
    UniConfPairList::Iter it(deltas);
    for (;;)
    {
        it.rewind();
        if (! it.next())
            break;

        UniConfKey key((*it).key());
        WvString value((*it).value());

        it.xunlink();
        dispatch_delta(key, value);
    }
}


void UniConfGen::dispatch_delta(const UniConfKey &key, WvStringParm value)
{
    cblist(key, value);
}


void UniConfGen::delta(const UniConfKey &key, WvStringParm value)
{
    if (hold_nesting == 0)
    {
        // not nested, dispatch immediately
        dispatch_delta(key, value);
    }
    else
    {
        hold_delta();
        deltas.add(new UniConfPair(key, value), true);
        unhold_delta();
    }
}


void UniConfGen::setv_naive(const UniConfPairList &pairs)
{
    UniConfPairList::Iter pair(pairs);
    for (pair.rewind(); pair.next(); )
	set(pair->key(), pair->value());
}


bool UniConfGen::haschildren(const UniConfKey &key)
{
    bool children = false;
    
    hold_delta();
    
    Iter *it = iterator(key);
    if (it)
    {
	it->rewind();
	if (it->next()) children = true;
	delete it;
    }
    
    unhold_delta();
    return children;
}


bool UniConfGen::exists(const UniConfKey &key)
{
    return !get(key).isnull();
}


int UniConfGen::str2int(WvStringParm value, int defvalue) const
{
    // also recognize bool strings as integers
    const char *strs[] = {
        "true", "yes", "on", "enabled",
        "false", "no", "off", "disabled"
    };
    const size_t numtruestrs = 4;

    if (!value.isnull())
    {
        // try to recognize an integer
        char *end;
        int num = strtol(value.cstr(), &end, 0);
        if (end != value.cstr())
            return num; // was a valid integer
        
        // try to recognize a special string
        for (size_t i = 0; i < sizeof(strs) / sizeof(const char*); ++i)
            if (strcasecmp(value, strs[i]) == 0)
                return i < numtruestrs;
    }
    return defvalue;
}


bool UniConfGen::isok()
{
    return true;
}


void UniConfGen::add_callback(void *cookie,
			      const UniConfGenCallback &callback)
{
    cblist.add(callback, cookie);
}


void UniConfGen::del_callback(void *cookie)
{
    cblist.del(cookie);
}



class _UniConfGenRecursiveIter : public IUniConfGen::Iter
{
    WvList<IUniConfGen::Iter> itlist;
    IUniConfGen *gen;
    UniConfKey top, current;
    bool sub_next;
    
public:
    _UniConfGenRecursiveIter(IUniConfGen *_gen, const UniConfKey &_top)
	: top(_top)
    {
	gen = _gen;
	sub_next = false;
    }
    
    virtual ~_UniConfGenRecursiveIter() { }

    virtual void rewind()
    {
	current = "";
	sub_next = false;
	itlist.zap();
	
	Iter *subi = gen->iterator(top);
	if (subi)
	{
	    subi->rewind();
	    itlist.prepend(subi, true);
	}
    }

    virtual bool next()
    {
	//assert(!itlist.isempty()); // trying to seek past the end is illegal!
	
	if (sub_next)
	{
	    sub_next = false;
	    
	    UniConfKey subkey(itlist.first()->key());
	    UniConfKey newkey(current, subkey);
	    //fprintf(stderr, "subiter: '%s'\n", newkey.cstr());
	    Iter *newsub = gen->iterator(UniConfKey(top, newkey));
	    if (newsub)
	    {
		current.append(subkey);
		//fprintf(stderr, "current is now: '%s'\n", current.cstr());
		newsub->rewind();
		itlist.prepend(newsub, true);
	    }
	}
	
	WvList<IUniConfGen::Iter>::Iter i(itlist);
	for (i.rewind(); i.next(); )
	{
	    if (i->next()) // NOTE: not the same as i.next()
	    {
		// set up so next time, we go into its subtree
		sub_next = true;
		return true;
	    }
	    
	    // otherwise, this iterator is empty; move up the tree
	    current = current.removelast();
	    //fprintf(stderr, "current is now: '%s'\n", current.cstr());
	    i.xunlink();
	}
	
	// all done!
	return false;
    }

    virtual UniConfKey key() const
    {
	//fprintf(stderr, "current is now: '%s'\n", current.cstr());
	if (!itlist.isempty())
	    return UniConfKey(current, itlist.first()->key());
	else
	    return current;
    }
    
    virtual WvString value() const
    {
	return gen->get(UniConfKey(top, key()));
    }
};


UniConfGen::Iter *UniConfGen::recursiveiterator(const UniConfKey &key)
{
    return new _UniConfGenRecursiveIter(this, key);
}


