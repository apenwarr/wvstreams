/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Defines a UniConfGen that manages a tree of UniConfGen instances.
 */
#ifndef __UNIMOUNTGEN_H
#define __UNIMOUNTGEN_H

#include "uniconfgen.h"
#include "wvcallback.h"
#include "wvmoniker.h"
#include "wvstringlist.h"


/** The UniMountTree implementation realized as a UniConfGen. */
class UniMountGen : public UniConfGen
{
protected:

    // Class to hold the generator with its mountpoint    
    class UniGenMount
    {
    public:
        UniGenMount(IUniConfGen *gen, const UniConfKey &key)
            : gen(gen), key(key) 
	    { }

        ~UniGenMount()
            { RELEASE(gen); }

        IUniConfGen *gen;
        UniConfKey key;
    };

    typedef class WvList<UniGenMount> MountList;
    MountList mounts;

public:
    /** Creates an empty UniConf tree with no mounted stores. */
    UniMountGen() 
        { }

    /** undefined. */
    UniMountGen(const UniMountGen &other);

    /** Destroys the UniConf tree along with all uncommitted data. */
    virtual ~UniMountGen()
        { }
    
    /**
     * Mounts a generator at a key using a moniker.
     
     * Returns the generator instance pointer, or NULL on failure.
     */
    virtual IUniConfGen *mount(const UniConfKey &key, WvStringParm moniker,
        bool refresh);
    
    /**
     * Mounts a generator at a key.
     * Takes ownership of the supplied generator instance.
     * 
     * "key" is the key
     * "gen" is the generator instance
     * "refresh" is if true, refreshes the generator after mount
     * Returns: the generator instance pointer, or NULL on failure
     */
    virtual IUniConfGen *mountgen(const UniConfKey &key, IUniConfGen *gen,
        bool refresh);

    /**
     * Unmounts the generator at a key and destroys it.
     *
     * "gen" is the generator instance
     * "commit" is if true, commits the generator before unmount
     */
    virtual void unmount(IUniConfGen *gen, bool commit);
    
    /**
     * Finds the generator that owns a key.
     * 
     * If the key exists, returns the generator that provides its
     * contents.  Otherwise returns the generator that would be
     * updated if a value were set.
     * 
     * "key" is the key
     * "mountpoint" is if not NULL, replaced with the mountpoint
     *        path on success
     * Returns: the handle, or a null handle if none
     */
    virtual IUniConfGen *whichmount(const UniConfKey &key,
				   UniConfKey *mountpoint);

    /** Determines if a key is a mountpoint. */
    virtual bool ismountpoint(const UniConfKey &key);
    
    /***** Overridden members *****/
    
    virtual bool exists(const UniConfKey &key);
    virtual bool haschildren(const UniConfKey &key);
    virtual WvString get(const UniConfKey &key);
    virtual void set(const UniConfKey &key, WvStringParm value);
    virtual bool refresh();
    virtual void commit();
    virtual Iter *iterator(const UniConfKey &key);
    virtual Iter *recursiveiterator(const UniConfKey &key);

private:
    /** Find the active generator for a given key. */
    UniGenMount *findmount(const UniConfKey &key);

    // Trim the key so it matches the generator starting point
    UniConfKey trimkey(const UniConfKey &foundkey, const UniConfKey &key)
        { return key.removefirst(foundkey.numsegments()); }

    /** Called by generators when a key changes. */
    void gencallback(const UniConfKey &key, WvStringParm value,
		     void *userdata);

    void makemount(const UniConfKey &key);
};

#endif //__UNIMOUNTGEN_H
