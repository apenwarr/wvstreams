/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 * 
 * A UniConf generator that replicates multiple generators, prioritized
 * by order.
 */
#include "uniconf.h"
#include "unireplicategen.h"
#include "wvmoniker.h"
#include "wvstringlist.h"
#include "wvtclstring.h"


//#define DPRINTF(format, args...) fprintf(stderr, format ,##args);
#define DPRINTF(format, args...)


// if 'obj' is non-NULL and is a UniConfGen, wrap that; otherwise wrap the
// given moniker.
static IUniConfGen *creator(WvStringParm s, IObject *obj, void *)
{
    IUniConfGenList gens;

    if (obj)
    {
        IUniConfGen *gen = mutate<IUniConfGen>(obj);
        if (gen)
            gens.append(gen, false);
    }
    if (gens.count() == 0)
    {
    	WvString encoded_monikers = wvtcl_unescape(s);
    	DPRINTF("encoded_monikers = %s\n", encoded_monikers.cstr());
    	WvStringList monikers;
    	wvtcl_decode(monikers, encoded_monikers);
    	DPRINTF("monikers = %s\n", monikers.join(",").cstr());
    	
    	WvStringList::Iter i(monikers);
    	for (i.rewind(); i.next(); )
    	{
            IUniConfGen *gen = wvcreate<IUniConfGen>(*i);
            if (gen)
            	gens.append(gen, false);
        }
    }

    return new UniReplicateGen(gens);
}

static WvMoniker<IUniConfGen> reg("replicate", creator);


/***** UniReplicateGen *****/

UniReplicateGen::UniReplicateGen()
{
}


UniReplicateGen::UniReplicateGen(const IUniConfGenList &_gens,
    	bool auto_free)
{
    IUniConfGenList::Iter i(_gens);
    
    for (i.rewind(); i.next(); )
    {
    	IUniConfGen *gen = i.ptr();
    	if (gen)
    	{
    	    gens.append(gen, auto_free);
            gen->setcallback(UniConfGenCallback(this,
            	&UniReplicateGen::deltacallback), gen);
        }
    }
    
    replicate();
}


void UniReplicateGen::prepend(IUniConfGen *gen, bool auto_free)
{
    if (gen)
    {
    	gens.prepend(gen, auto_free);
        gen->setcallback(UniConfGenCallback(this,
            &UniReplicateGen::deltacallback), gen);
            
        replicate();
    }
}


void UniReplicateGen::append(IUniConfGen *gen, bool auto_free)
{
    if (gen)
    {
    	gens.append(gen, auto_free);
        gen->setcallback(UniConfGenCallback(this,
            &UniReplicateGen::deltacallback), gen);
            
        replicate();
    }
}


bool UniReplicateGen::isok()
{
    IUniConfGenList::Iter i(gens);
    for (i.rewind(); i.next(); )
    {
    	if (i->isok())
    	    return true;
    }
    
    return false;
}


bool UniReplicateGen::refresh()
{
    bool result = true;
    
    IUniConfGenList::Iter i(gens);
    for (i.rewind(); i.next(); )
    {
    	if (!i->refresh())
    	    result = false;
    }
    
    return result;
}


void UniReplicateGen::commit()
{
    IUniConfGenList::Iter i(gens);
    for (i.rewind(); i.next(); )
    	i->commit();
}


void UniReplicateGen::deltacallback(const UniConfKey &key, WvStringParm value,
                                void *userdata)
{
    DPRINTF("UniReplicateGen::deltacallback(%s, %s)\n",
    	    key.cstr(), value.cstr());

    static bool processing_callback = false;
    if (!processing_callback)
    {
    	processing_callback = true;
    	
    	IUniConfGen *src_gen = static_cast<IUniConfGen *>(userdata);
    
    	IUniConfGenList::Iter j(gens);
    	for (j.rewind(); j.next(); )
    	{
    	    if (!j->isok())
    	    	continue;
    	    	
    	    if (j.ptr() != src_gen)
    	    	j->set(key, value);
    	}
        
    	delta(key, value);
    	
    	processing_callback = false;
    }
}


void UniReplicateGen::set(const UniConfKey &key, WvStringParm value)
{
    DPRINTF("UniReplicateGen::set(%s, %s)\n",
    	    key.cstr(), value.cstr());
    
    IUniConfGen *first = first_ok();
    if (first)
    	first->set(key, value);
    else
    	DPRINTF("UniReplicateGen::set: first == NULL\n");
}


WvString UniReplicateGen::get(const UniConfKey &key)
{
    IUniConfGen *first = first_ok();
    if (first)
    	return first->get(key);
    else
    	return WvString::null;
}


UniConfGen::Iter *UniReplicateGen::iterator(const UniConfKey &key)
{
    IUniConfGen *first = first_ok();
    if (first)
    	return first->iterator(key);
    else
    	return NULL;
}


IUniConfGen *UniReplicateGen::first_ok() const
{
    IUniConfGenList::Iter j(gens);
    for (j.rewind(); j.next(); )
    	if (j->isok())
    	    return j.ptr();
    	    
    return NULL;
}


void UniReplicateGen::replicate(const UniConfKey &key)
{
    DPRINTF("UniReplicateGen::replicate(%s)\n", key.cstr());
    
    
    hold_delta();
    
    IUniConfGen *first = first_ok();
    
    IUniConfGenList::Iter j(gens);
    for (j.rewind(); j.next(); )
    {
    	if (!j->isok())
    	    continue;
    	    
    	UniConfGen::Iter *i = j->recursiveiterator(key);
    	if (!i) return;
    
    	for (i->rewind(); i->next(); )
    	{
            WvString value(i->value());
    	
	    DPRINTF("UniReplicateGen::replicate: key=%s, value=%s\n",
	    	    i->key().cstr(), value.cstr());

    	    if (j.ptr() == first)
    	    {
    	    	deltacallback(i->key(), value, first);
    	    }
    	    else
    	    {
    	    	if (!first->exists(i->key()))
    	    	    first->set(i->key(), value);
    	    }
    	}
    
    	delete i;
    }
    
    unhold_delta();
}
