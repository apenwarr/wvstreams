/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 * 
 * A totally evil UniConfGen that "unwraps" a UniConf object by turning it
 * back into a UniConfGen.  See uniunwrapgen.h.
 */
#include "uniunwrapgen.h"


UniUnwrapGen::UniUnwrapGen(const UniConf &inner)
{
    refreshing = committing = false;
    setinner(inner);
}


UniUnwrapGen::~UniUnwrapGen()
{
    xinner.del_callback(this, true);
}


void UniUnwrapGen::setinner(const UniConf &inner)
{
    if (xinner.rootobj())
	xinner.del_callback(this, true);
    xinner = inner;
    if (xinner.rootobj())
	xinner.add_callback(this,
			    UniConfCallback(this, &UniUnwrapGen::gencallback),
			    true);
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
    xinner[key].prefetch(recursive);
}


WvString UniUnwrapGen::get(const UniConfKey &key)
{
    return xinner[key].getme();
}


void UniUnwrapGen::set(const UniConfKey &key, WvStringParm value)
{
    xinner[key].setme(value);
}


bool UniUnwrapGen::exists(const UniConfKey &key)
{
    return xinner[key].exists();
}


bool UniUnwrapGen::haschildren(const UniConfKey &key)
{
    return xinner[key].haschildren();
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
    return new Iter(xinner[key]);
}


UniConfGen::Iter *UniUnwrapGen::recursiveiterator(const UniConfKey &key)
{
    return new RecursiveIter(xinner[key]);
}


void UniUnwrapGen::gencallback(const UniConf &cfg, const UniConfKey &key)
{
    delta(cfg[key].fullkey(), cfg[key].getme());
}
