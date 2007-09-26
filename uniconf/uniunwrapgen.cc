/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 * 
 * A totally evil UniConfGen that "unwraps" a UniConf object by turning it
 * back into a UniConfGen.  See uniunwrapgen.h.
 */
#include "uniconfroot.h"
#include "uniunwrapgen.h"
#include "wvlinkerhack.h"

WV_LINK(UniUnwrapGen);


UniUnwrapGen::UniUnwrapGen(const UniConf &inner)
{
    refreshing = committing = false;
    setinner(inner);
}


UniUnwrapGen::~UniUnwrapGen()
{
    UniConfRoot *root = xinner.rootobj();
    if (root)
	root->mounts.del_callback(this);
}


void UniUnwrapGen::setinner(const UniConf &inner)
{
    UniConfRoot *root = xinner.rootobj();
    if (root)
	root->mounts.del_callback(this);

    xinner = inner;
    xfullkey = xinner.fullkey();

    root = xinner.rootobj();
    if (root)
	root->mounts.add_callback(this, wv::bind(&UniUnwrapGen::gencallback,
						 this, wv::_1, wv::_2));
}


UniConf UniUnwrapGen::_sub(const UniConfKey &key)
{
    if (key.isempty())
	return xinner;
    else
	return xinner[key];
}


void UniUnwrapGen::commit()
{
    if (!committing)
    {
	committing = true;
	xinner.commit();
	committing = false;
    }
}


bool UniUnwrapGen::refresh()
{
    if (!refreshing)
    {
	refreshing = true;
	bool ret = xinner.refresh();
	refreshing = false;
	return ret;
    }
    return true;
}


void UniUnwrapGen::prefetch(const UniConfKey &key, bool recursive)
{
    _sub(key).prefetch(recursive);
}


WvString UniUnwrapGen::get(const UniConfKey &key)
{
    return _sub(key).getme();
}


void UniUnwrapGen::set(const UniConfKey &key, WvStringParm value)
{
    _sub(key).setme(value);
}


void UniUnwrapGen::setv(const UniConfPairList &pairs)
{
    // Extremely evil.  This pokes directly into UniMountGen, because we
    // don't want to expose setv to users.
    xinner.rootobj()->mounts.setv(pairs);
}


bool UniUnwrapGen::exists(const UniConfKey &key)
{
    return _sub(key).exists();
}


bool UniUnwrapGen::haschildren(const UniConfKey &key)
{
    return _sub(key).haschildren();
}


bool UniUnwrapGen::isok()
{
    IUniConfGen *gen = xinner.whichmount();
    return gen ? gen->isok() : false;
}


class UniUnwrapGen::Iter : public UniConfGen::Iter
{
    UniConf::Iter i;
    
public:
    Iter(const UniConf &cfg)
	: i(cfg)
        { }
    virtual ~Iter()
        { }
    
    /***** Overridden members *****/
    virtual void rewind() { i.rewind(); }
    virtual bool next() { return i.next(); }
    virtual UniConfKey key() const { return i->key(); }
    virtual WvString value() const { return i->getme(); }
};


class UniUnwrapGen::RecursiveIter : public UniConfGen::Iter
{
    UniConf::RecursiveIter i;
    
public:
    RecursiveIter(const UniConf &cfg)
	: i(cfg)
        { }
    virtual ~RecursiveIter()
        { }
    
    /***** Overridden members *****/
    virtual void rewind() { i.rewind(); }
    virtual bool next() { return i.next(); }
    virtual UniConfKey key() const { return i->key(); }
    virtual WvString value() const { return i->getme(); }
};


UniConfGen::Iter *UniUnwrapGen::iterator(const UniConfKey &key)
{
    return new Iter(_sub(key));
}


UniConfGen::Iter *UniUnwrapGen::recursiveiterator(const UniConfKey &key)
{
    return new RecursiveIter(_sub(key));
}


void UniUnwrapGen::gencallback(const UniConfKey &key, WvStringParm value)
{
    UniConfKey subkey;
    if (xfullkey.suborsame(key, subkey))
	delta(subkey, value);
}
