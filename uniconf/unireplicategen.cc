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
#include "wvlinkerhack.h"

WV_LINK(UniReplicateGen);


#if 0
#define DPRINTF(format, args...) fprintf(stderr, format ,##args);
#else
#define DPRINTF if (0) printf
#endif


static IUniConfGen *creator(WvStringParm s)
{
    IUniConfGenList gens;

    if (gens.isempty())
    {
    	DPRINTF("encoded_monikers = %s\n", s.cstr());
    	WvStringList monikers;
    	wvtcl_decode(monikers, s);
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
            gen->gen->add_callback(this,
				   wv::bind(&UniReplicateGen::deltacallback,
					    this, gen, wv::_1, wv::_2));
        }
    }
    
    replicate();
}


UniReplicateGen::~UniReplicateGen()
{
    GenList::Iter i(gens);
    for (i.rewind(); i.next(); )
	i->gen->del_callback(this);
}


void UniReplicateGen::prepend(IUniConfGen *_gen, bool auto_free)
{
    Gen *gen = new Gen(_gen, auto_free);
    if (gen)
    {
    	gens.prepend(gen, true);
        gen->gen->add_callback(this, wv::bind(&UniReplicateGen::deltacallback,
					      this, gen, wv::_1, wv::_2));
            
        replicate();
    }
}


void UniReplicateGen::append(IUniConfGen *_gen, bool auto_free)
{
    Gen *gen = new Gen(_gen, auto_free);
    if (gen)
    {
    	gens.append(gen, true);
        gen->gen->add_callback(this, wv::bind(&UniReplicateGen::deltacallback,
					      this, gen, wv::_1, wv::_2));
            
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


void UniReplicateGen::deltacallback(Gen *src_gen, const UniConfKey &key,
				    WvStringParm value)
{
    DPRINTF("UniReplicateGen::deltacallback(%s, %s)\n",
            key.printable().cstr(), value.cstr());

    if (!processing_callback)
    {
    	DPRINTF("UniReplicateGen::deltacallback(): !processing_callback\n");

    	processing_callback = true;
    	
    	GenList::Iter j(gens);
    	for (j.rewind(); j.next(); )
    	{
    	    if (!j->isok())
    	    	continue;
    	    	
    	    if (j.ptr() != src_gen)
            {
                DPRINTF("UniReplicateGen::deltacallback: %p->set(%s, %s)\n",
                        j.ptr(), key.printable().cstr(), value.cstr());
    	    	j->gen->set(key, value);
            }
    	}
        
    	delta(key, value);
    	
    	processing_callback = false;
    }
    else
    {
    	DPRINTF("UniReplicateGen::deltacallback(): processing_callback\n");
    }
}


void UniReplicateGen::set(const UniConfKey &key, WvStringParm value)
{
    DPRINTF("UniReplicateGen::set(%s, %s)\n",
    	    key.printable().cstr(), value.cstr());
    
    replicate_if_any_have_become_ok();
    
    Gen *first = first_ok();
    if (first)
    	first->gen->set(key, value);
    else
    	DPRINTF("UniReplicateGen::set: first == NULL\n");
}


void UniReplicateGen::setv(const UniConfPairList &pairs)
{
    DPRINTF("UniReplicateGen::setv\n");

    replicate_if_any_have_become_ok();

    Gen *first = first_ok();
    if (first)
	first->gen->setv(pairs);
    else
	DPRINTF("UniReplicateGen::setv: first == NULL\n");
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
    DPRINTF("UniReplicateGen::replicate(%s)\n", key.printable().cstr());
       
    hold_delta();
    
    Gen *first = first_ok();
    
    GenList::Iter j(gens);
    for (j.rewind(); j.next(); )
    {
        DPRINTF("UniReplicateGen::replicate: %p\n", j.ptr());
        
    	if (!j->isok())
        {
            DPRINTF("UniReplicateGen::replicate: !isok()\n");
    	    continue;
        }
    	    
    	UniConfGen::Iter *i = j->gen->recursiveiterator(key);
    	if (!i)
        {
            DPRINTF("UniReplicateGen::replicate: no iterator\n");
            continue;
        }
    
    	for (i->rewind(); i->next(); )
    	{
	    DPRINTF("UniReplicateGen::replicate: key=%s, value=%s\n",
	    	    i->key().printable().cstr(), i->value().cstr());

    	    if (j.ptr() == first)
    	    {
                DPRINTF("UniReplicateGen::replicate: deltacallback()\n");
    	    	deltacallback(first, i->key(), i->value());
    	    }
    	    else
    	    {
    	    	if (!first->gen->exists(i->key()))
                {
                    DPRINTF("UniReplicateGen::replicate: !exists()\n");
    	    	    first->gen->set(i->key(), i->value());
                }
                else
                {
                    DPRINTF("UniReplicateGen::replicate: exists()\n");
                }
    	    }
    	}
    
    	delete i;
    }
    
    unhold_delta();

    DPRINTF("UniReplicateGen::replicate: done\n");
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

