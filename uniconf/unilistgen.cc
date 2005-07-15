/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 *
 * UniListGen is a UniConf generator to allow multiple generators to be
 * stacked in a priority sequence for get/set/etc.
 * 
 */
#include "unilistgen.h"
#include "wvmoniker.h"
#include "wvtclstring.h"
#include "wvstringlist.h"
#include "wvlinkerhack.h"

WV_LINK(UniListGen);



class UniListGen::IterIter : public UniConfGen::Iter
{
protected:
    DeclareWvScatterTable(UniConfKey);
    DeclareWvList2(IterList, UniConfGen::Iter);
    
    IterList l;
    IterList::Iter *i;
    UniConfKeyTable d;
    
public:
    IterIter(UniListGen *gen, const UniConfKey &key);
    virtual ~IterIter() { delete i; }
    
    virtual void rewind();
    virtual bool next();
    virtual UniConfKey key() const;
    virtual WvString value() const;
};



// if 'obj' is non-NULL and is a UniConfGen then whoever invoked this is being
// silly. we'll make a list and add the single generator to it anyways, for the
// sake of not breaking things
//
// otherwise, treat the moniker as a tcl list of monikers and add the generator
// made by each moniker to the generator list
static IUniConfGen *creator(WvStringParm s, IObject *obj, void *)
{
    UniConfGenList *l = new UniConfGenList();
    IUniConfGen *gen = NULL;

    if (obj)
    {
        gen = mutate<IUniConfGen>(obj);
        if (gen)
            l->append(gen, true);
    }

    if (!gen)
    {
        WvStringList gens;
        wvtcl_decode(gens, s);
        WvStringList::Iter i(gens);

        for (i.rewind(); i.next();)
        {
            gen = wvcreate<IUniConfGen>(i());
            if (gen)
                l->append(gen, true);
        }
    }

    return new UniListGen(l);
}
 
static WvMoniker<IUniConfGen> reg("list", creator);

UniListGen::UniListGen(UniConfGenList *_l) : l(_l)
{
    UniConfGenList::Iter i(*l);
    for (i.rewind(); i.next(); )
        i->add_callback(this, 
		UniConfGenCallback(this, &UniListGen::gencallback), NULL);
}


UniListGen::~UniListGen()
{
    UniConfGenList::Iter i(*l);
    for (i.rewind(); i.next(); )
        i->del_callback(this);
    delete l;
}


void UniListGen::commit()
{
    UniConfGenList::Iter i(*l);
    for (i.rewind(); i.next();)
        i->commit();
}


bool UniListGen::refresh()
{
    bool result = true;

    UniConfGenList::Iter i(*l);
    for (i.rewind(); i.next();)
        result = i->refresh() && result;
    return result;
}


WvString UniListGen::get(const UniConfKey &key)
{
    UniConfGenList::Iter i(*l);
    for (i.rewind(); i.next(); )
        if (i->exists(key))
            return i->get(key);
    return WvString::null;
}


// we try to set *all* our generators.  Read-only ones will ignore us.
void UniListGen::set(const UniConfKey &key, WvStringParm value)
{
    UniConfGenList::Iter i(*l);
    for (i.rewind(); i.next(); )
	i->set(key, value);
}


void UniListGen::setv(const UniConfPairList &pairs)
{
    UniConfGenList::Iter i(*l);
    for (i.rewind(); i.next(); )
	i->setv(pairs);
}


bool UniListGen::exists(const UniConfKey &key)
{
    UniConfGenList::Iter i(*l);
    for (i.rewind(); i.next();)
    {
        if (i->exists(key))
            return true;
    }
    return false;
}


bool UniListGen::haschildren(const UniConfKey &key)
{
    UniConfGenList::Iter i(*l);
    for (i.rewind(); i.next();)
    {
        if (i->haschildren(key))
            return true;
    }
    return false;
}


bool UniListGen::isok()
{
    UniConfGenList::Iter i(*l);
    for (i.rewind(); i.next();)
    {
        if (!i->isok())
            return false;
    }
    return true;
}


void UniListGen::gencallback(const UniConfKey &key, WvStringParm value, void *userdata)
{
    delta(key, get(key));
}


UniConfGen::Iter *UniListGen::iterator(const UniConfKey &key)
{
    return new IterIter(this, key);
}


/***** UniListGen::IterIter *****/

UniListGen::IterIter::IterIter(UniListGen *gen, const UniConfKey &key)
{
    UniConfGenList::Iter geniter(*gen->l);
    for (geniter.rewind(); geniter.next(); )
    {
	Iter *it = geniter->iterator(key);
	if (it)
	    l.append(it, true);
    }

    i = new IterList::Iter(l);
}


void UniListGen::IterIter::rewind()
{
    for ((*i).rewind(); (*i).next(); )
        (*i)->rewind();

    i->rewind();
    i->next();

    d.zap();
}


bool UniListGen::IterIter::next()
{
    if (l.isempty())
        return false;

    if ((*i)->next())
    {
        // When iterating, make sure each key value is only returned once
        // (from the top item in the list)
        if (!d[(*i)->key()])
        {
            d.add(new UniConfKey((*i)->key()), true);
            return true;
        }
        else
            return next();
    }

    if (!i->next())
        return false;

    return next();
}


UniConfKey UniListGen::IterIter::key() const
{
    return (*i)->key();
}


WvString UniListGen::IterIter::value() const
{
    return (*i)->value();
}


