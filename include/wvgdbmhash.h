/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2003 Net Integration Technologies, Inc.
 *
 * A hash table container backed by a gdbm database.
 */
#ifndef __WVGDBMHASH_H
#define __WVGDBMHASH_H

#include "wvautoconf.h"

#ifndef HAVE_GDBM_H
# error "Sorry, no gdbm support in wvstreams!"
#endif

#include <wvhashtable.h>
#include <wvstring.h>
#include <wvbuf.h>

#include <gdbm.h>

// Use this to cheat our way into autocasting different types to a datum
class DatumAdapterBase
{
public:
    operator datum()
        { return mydatum; }
    datum mydatum;
};


template <class D>
class DatumAdapter : public DatumAdapterBase
{
public:
    typedef D* ReturnType;
    static ReturnType convert(datum convertme)
        { return (ReturnType)convertme.dptr; }
    DatumAdapter(const D &d)
        { mydatum.dptr = (char *)&d; mydatum.dsize = sizeof(D); }
    DatumAdapter(const D *d)
        { mydatum.dptr = (char *)d; mydatum.dsize = sizeof(*d); }
};


// Provide specializations for some common types
template <>
class DatumAdapter<WvString> : public DatumAdapterBase
{
public:
    typedef WvString ReturnType;
    operator WvString ()
        { return WvString(mydatum.dptr); }
    static ReturnType convert(datum convertme)
        { return WvString(convertme.dptr); }
    DatumAdapter(WvStringParm s) 
        { mydatum.dptr = (char *)s.cstr();
          mydatum.dsize = s.len() + 1; }
};


// Base class for the template to save space
class WvGdbmHashBase
{
public:
    WvGdbmHashBase(WvStringParm dbfile);
    ~WvGdbmHashBase();
    int count() const { return entries; }
    bool isempty() const { return !entries; }

    int add(datum key, datum data, bool replace);
    int remove(datum key);
    datum find(datum key);
    bool exists(datum key);
    
    class IterBase
    {
    public:
        IterBase(WvGdbmHashBase &_gdbmhash);
        ~IterBase();
        void rewind();
        void next();
        
    protected:
        WvGdbmHashBase &gdbmhash;
        datum curkey, nextkey;
        datum curdata;
    };
private:
    friend class IterBase;
    int entries;
    GDBM_FILE dbf;
};


/**
 * This hashtable is different from normal WvStreams hashtables in that it
 * stores the data on disk.
 * This affects memory managment for objects stored in it.
 * For find, the returned object is only guaranteed to be around until the
 * next find/or next() for iterators.
 */
template <class K, class D>
class WvGdbmHash
    : public WvGdbmHashBase
{
public:
    WvGdbmHash(WvStringParm dbfile)
        : WvGdbmHashBase(dbfile)
        { saveddata.dptr = NULL; }

    void add(const K &key, const D &data, bool replace = false)
    {
        int r = WvGdbmHashBase::add(DatumAdapter<K>(key),
				    DatumAdapter<K>(data), replace);
        assert(!r && "Set the replace flag to replace existing elements.");
    }

    void remove(const K &key)
        { WvGdbmHashBase::remove(DatumAdapter<K>(key)); }

    typename DatumAdapter<D>::ReturnType find(const K &key)
    {   
        free(saveddata.dptr);

        saveddata = WvGdbmHashBase::find(DatumAdapter<K>(key));

        return DatumAdapter<D>::convert(saveddata);
    }

    typename DatumAdapter<D>::ReturnType operator[] (const K &key)
        { return find(key); }
        
    bool exists(const K & key)
        { return WvGdbmHashBase::exists(DatumAdapter<K>(key)); }

    class Iter : public WvGdbmHashBase::IterBase
    {
    public:
        Iter(WvGdbmHash &_gdbmhash) : IterBase(_gdbmhash) 
	    {}

        D *next()
        {
            if (!nextkey.dptr)
                return NULL;
            IterBase::next();
            return (D *)curdata.dptr;
        }
        
	D *cur()
        {
            return (D *)curdata.dptr;
        }
	
	typename DatumAdapter<K>::ReturnType key()
	    { return DatumAdapter<K>::convert(curkey); }
	
	// FIXME: this is a non-standard wvutils iterator.  We should find
	// a way to use WvIterStuff in wvlink.h.
        typename DatumAdapter<D>::ReturnType operator()()
            { return DatumAdapter<D>::convert(curdata); }
    };

protected:
    datum saveddata;
};

#endif // __WVGDBMHASH_H
