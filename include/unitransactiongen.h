/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 *
 * A UniConfGen that represents pending transactions to another generator.
 */
#ifndef __UNITRANSACTIONGEN_H
#define __UNITRANSACTIONGEN_H

#include "uniconfgen.h"

class UniConfChangeTree;
class UniConfValueTree;

/**
 * A UniConfGen that represents pending transactions to another generator.
 * It's moniker is "transaction" followed by a moniker to create
 * the underlying generator, or alternatively an IUniConfGen object.
 * 
 * A set() call on a UniTransactionGen records the fact that you wish to
 * perform that set() call on the underlying generator.
 *
 * When you call commit(), the underlying generator is modified to 
 * incorporate all set() calls since the last commit() or refresh() (or
 * since creation if none) as if they had all been made on the underlying
 * generator at the moment of your call to commit(). However, commit()
 * does this in a way that prevents unnecessary extra callbacks from
 * being issued by the underlying generator.
 *
 * When you call refresh(), all set() calls since the last commit() or
 * refresh() (or since creation if none) are discarded.
 *
 * When you use get(), exists(), haschildren(), iterator(), or
 * recursiveiterator(), the results that you get are equivalent to the results
 * that you would have gotten if you had called commit() and then used these
 * methods on the underlying generator, except that they do not
 * modify the underlying generator.
 *
 * Note that the UniTransactionGen's iterators might iterate over keys 
 * in a different order than would the underlying generator's iterators
 * after a commit(). Also, the UniTransactionGen's iterators presently do
 * not support concurrent modification (regardless of whether the underlying
 * generator's iterators do).
 *
 * The UniTransactionGen properly issues callbacks due to calls to set()
 * and refresh() and also due to changes to the underlying generator from
 * other sources.
 *
 * If another source deletes a tree from the underlying generator that you
 * were not already planning to delete or replace, then the
 * UniTransactionGen currently has to do some very ugly callbacks. (See the
 * FIXME in the .cc file for details.) In all other cases, if the underlying
 * generator issues precisely the callbacks needed to indicate a change in
 * state, then so will the UniTransactionGen.
 *
 * In order to work properly, the UniTransactionGen has to assume that the
 * underlying generator completely obeys the UniConfGen semantics. If it does
 * not, then the UniTransactionGen might not obey its own class contract or
 * even the UniConfGen semantics either. Incidentally though, breaking
 * callbacks in the underlying generator cannot break anything in the
 * UniTransactionGen other than its own callbacks.
 *
 * Using a UniTransactionGen and/or its underlying generator in multiple
 * threads will probably break it.
 *
 * Though similar in concept to a UniFilterGen, the UniTransactionGen
 * doesn't derive from it because we have basically no need for any of
 * its functionality.
 */
class UniTransactionGen : public UniConfGen
{
public:
    /** 
     * Constructs a UniTransactionGen for the given underlying generator,
     * which must be non-null.
     */
    UniTransactionGen(IUniConfGen *_base);

    /**
     * Destroys the UniTransactionGen and the underlying generator. Does
     * not commit uncommitted data.
     */
    ~UniTransactionGen();


    /***** Overridden methods *****/
    
    WvString get(const UniConfKey &key);
    void set(const UniConfKey &key, WvStringParm value);
    void commit();
    bool refresh();
    Iter *iterator(const UniConfKey &key);
    bool isok();
    void flush_buffers();
    
protected:
    UniConfChangeTree *root;
    IUniConfGen *base;

    /**
     * A recursive helper functions for commit().
     */
    void apply_changes(UniConfChangeTree *node,
		       const UniConfKey &section);

    /**
     * A recursive helper functions for apply_changes().
     */
    void apply_values(UniConfValueTree *newcontents,
		      const UniConfKey &section);

    /**
     * A recursive helper function for refresh().
     */
    void cancel_changes(UniConfChangeTree *node,
			const UniConfKey &section);

    /**
     * A recursive helper function for cancel_changes().
     */
    void cancel_values(UniConfValueTree *newcontents,
		       const UniConfKey &section);

    /**
     * The callback function for the underlying generator.
     */
    void gencallback(const UniConfKey &key,
		     WvStringParm value,
		     void *userdata);

    /**
     * A recursive helper function for gencallback().
     */
    void was_removed(UniConfChangeTree *node,
		     const UniConfKey &key);

    /**
     * A recursive helper function for the other was_removed().
     */
    void was_removed(UniConfValueTree *node,
		     const UniConfKey &key);

    /**
     * Four functions to implement the functionality of set() so
     * that it isn't two pages long.
     */
    UniConfValueTree *create_value(UniConfValueTree *parent,
				   const UniConfKey &key,
				   int seg,
				   WvStringParm value);

    UniConfChangeTree *create_change(UniConfChangeTree *parent,
				     const UniConfKey &key,
				     int seg,
				     WvStringParm value);

    UniConfValueTree *set_value(UniConfValueTree *node,
				const UniConfKey &key,
				int seg,
				WvStringParm value);

    UniConfChangeTree *set_change(UniConfChangeTree *node,
				  const UniConfKey &key,
				  int seg,
				  WvStringParm value);
};

#endif
