/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2003 Net Integration Technologies, Inc.
 *
 * A hash table container backed by a gdbm database.
 */

#ifdef WITH_GDBM

#ifndef WVGDBMHASH_H
#define WVGDBMHASH_H

#include <wvhashtable.h>
#include <wvstring.h>

#include <gdbm.h>

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
        datum nextkey;
        datum curdata;
    };
private:
    friend class IterBase;
    int entries;
    GDBM_FILE dbf;
};

// Use this to cheat our way into autocasting different types to a datum
class DatumAdapterBase
{
public:
    operator datum ()
        { return mydatum; }
    datum mydatum;
};

template <class D>
class DatumAdapter : public DatumAdapterBase
{
public:
    typedef D* ReturnType;
    static ReturnType convert(char *data)
        { return (ReturnType)data; }
    operator const D& ()
        { return *((D*)mydatum.dptr); }
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
    static ReturnType convert(char *data)
        { return WvString(data); }
    DatumAdapter(WvStringParm s) 
        { mydatum.dptr = (char *)s.cstr();
          mydatum.dsize = s.len() + 1; }
};

/**
 *  This hashtable is different from normal WvStreams hashtables in that is
 *  stores the data on disk.
 *  This affects memory managment for objects stored in it.
 *  For find, the returned object is only guaranteed to be around until the
 *  next find/or next() for iterators.
 */

template <class K, class D>
class WvGdbmHash
    : public WvGdbmHashBase
{
public:
    WvGdbmHash(WvStringParm dbfile)
        : WvGdbmHashBase(dbfile)
        { saveddata.dptr = NULL; }

    void add(DatumAdapter<K> key, DatumAdapter<D> data, bool replace = false)
    {
        int r = WvGdbmHashBase::add(key, data, replace);
        assert(!r && "Set the replace flag to replace existing elements.");
    }

    void remove(DatumAdapter<K> key)
        { WvGdbmHashBase::remove(key); }

    typename DatumAdapter<D>::ReturnType find(DatumAdapter<K> key)
    {   
        free(saveddata.dptr);

        saveddata = WvGdbmHashBase::find(key);

        return DatumAdapter<D>::convert(saveddata.dptr);
    }
        
    bool exists(DatumAdapter<K> key)
        { return WvGdbmHashBase::exists(key); }

    // Warning: Iterators are thoroughly untested (mag: 15/04/03)
    class Iter : public WvGdbmHashBase::IterBase
    {
    public:
        Iter(WvGdbmHash &_gdbmhash) : IterBase(_gdbmhash) {};
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
        typename DatumAdapter<D>::ReturnType operator()()
            { return DatumAdapter<D>::convert(curdata); }
    };

protected:
    datum saveddata;
};


#endif

#endif
