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


#define DPRINTF(format, args...) fprintf(stderr, format ,##args);
//#define DPRINTF(format, args...)


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

UniReplicateGen::UniReplicateGen() : processing_callback(false)
{
}


UniReplicateGen::UniReplicateGen(const IUniConfGenList &_gens,
    	bool auto_free) : processing_callback(false)
{
    IUniConfGenList::Iter i(_gens);
    
    for (i.rewind(); i.next(); )
    {
    	Gen *gen = new Gen(i.ptr(), auto_free);
    	if (gen)
    	{
    	    gens.append(gen, true);
            gen->gen->setcallback(UniConfGenCallback(this,
            	&UniReplicateGen::deltacallback), gen);
        }
    }
    
    replicate();
}


void UniReplicateGen::prepend(IUniConfGen *_gen, bool auto_free)
{
    Gen *gen = new Gen(_gen, auto_free);
    if (gen)
    {
    	gens.prepend(gen, true);
        gen->gen->setcallback(UniConfGenCallback(this,
            &UniReplicateGen::deltacallback), gen);
            
        replicate();
    }
}


void UniReplicateGen::append(IUniConfGen *_gen, bool auto_free)
{
    Gen *gen = new Gen(_gen, auto_free);
    if (gen)
    {
    	gens.append(gen, true);
        gen->gen->setcallback(UniConfGenCallback(this,
            &UniReplicateGen::deltacallback), gen);
            
        replicate();
    }
}


bool UniReplicateGen::isok()
{
    return first_ok() != NULL;
}


bool UniReplicateGen::refresh()
{
    bool result = true;
    
    replicate_if_any_have_become_ok();
    
    GenList::Iter i(gens);
    for (i.rewind(); i.next(); )
    {
    	if (!i->gen->refresh())
    	    result = false;
    }
    
    return result;
}


void UniReplicateGen::commit()
{
    replicate_if_any_have_become_ok();
    
    GenList::Iter i(gens);
    for (i.rewind(); i.next(); )
    {
    	i->gen->commit();
    }
}


void UniReplicateGen::deltacallback(const UniConfKey &key, WvStringParm value,
                                void *userdata)
{
    if (!processing_callback)
    {
    	DPRINTF("UniReplicateGen::deltacallback(%s, %s)\n",
    	    	key.cstr(), value.cstr());

    	processing_callback = true;
    	
    	Gen *src_gen = static_cast<Gen *>(userdata);
    
    	GenList::Iter j(gens);
    	for (j.rewind(); j.next(); )
    	{
    	    if (!j->isok())
    	    	continue;
    	    	
    	    if (j.ptr() != src_gen)
    	    	j->gen->set(key, value);
    	}
        
    	delta(key, value);
    	
    	processing_callback = false;
    }
}


void UniReplicateGen::set(const UniConfKey &key, WvStringParm value)
{
    DPRINTF("UniReplicateGen::set(%s, %s)\n",
    	    key.cstr(), value.cstr());
    
    replicate_if_any_have_become_ok();
    
    Gen *first = first_ok();
    if (first)
    	first->gen->set(key, value);
    else
    	DPRINTF("UniReplicateGen::set: first == NULL\n");
}


WvString UniReplicateGen::get(const UniConfKey &key)
{
    for (;;)
    {
    	replicate_if_any_have_become_ok();
    
    	Gen *first = first_ok();
    	if (first)
    	{
    	    WvString result = first->gen->get(key);

    	    // It's possible that first has become !isok(); we must
    	    // take care of this case carefully
    	    if (!result && !first->isok())
    	    {
    	    	Gen *new_first = first_ok();
    	    	if (new_first == first)
    	    	    return result;
    	    	first = new_first; 
    	    }
    	    else
    	    	return result;
    	}
    	else
    	    return WvString::null;
    }
}


UniConfGen::Iter *UniReplicateGen::iterator(const UniConfKey &key)
{
    replicate_if_any_have_become_ok();
    
    Gen *first = first_ok();
    if (first)
    	return first->gen->iterator(key);
    else
    	return NULL;
}


UniReplicateGen::Gen *UniReplicateGen::first_ok() const
{
    GenList::Iter j(gens);
    for (j.rewind(); j.next(); )
    {
    	if (j->isok())
    	    return j.ptr();
    }
    	    
    return NULL;
}


void UniReplicateGen::replicate(const UniConfKey &key)
{
    DPRINTF("UniReplicateGen::replicate(%s)\n", key.cstr());
       
    hold_delta();
    
    Gen *first = first_ok();
    
    GenList::Iter j(gens);
    for (j.rewind(); j.next(); )
    {
    	if (!j->isok())
    	    continue;
    	    
    	UniConfGen::Iter *i = j->gen->recursiveiterator(key);
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
    	    	if (!first->gen->exists(i->key()))
    	    	    first->gen->set(i->key(), value);
    	    }
    	}
    
    	delete i;
    }
    
    unhold_delta();
}

void UniReplicateGen::replicate_if_any_have_become_ok()
{
    bool should_replicate = false;
    
    GenList::Iter j(gens);
    for (j.rewind(); j.next(); )
    {
    	if (!j->was_ok && j->gen->isok())
    	{
    	    j->was_ok = true;
    	    
    	    should_replicate = true;
    	}
    }
    
    if (should_replicate)
    {
    	DPRINTF("UniReplicateGen::replicate_if_any_have_become_ok: replicating\n");
    	replicate();
    }
}

