/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2003 Net Integration Technologies, Inc.
 *
 * A hash table container backed by a Berkeley DB (libdb) database.
 * Intended to work with versions as old as libdb1.
 * 
 * FIXME: the error checking in here is horrendous.
 * 
 * FIXME: share more code with WvGdbmHash, since there's a lot of duplication?
 */
#ifndef __WVBDBHASH_H
#define __WVBDBHASH_H

#include "wvautoconf.h"

#ifndef HAVE_GDBM_H
# error "Sorry, no libdb support in wvstreams!"
#endif

#include "wvhashtable.h"
#include "wvserialize.h"

// Base class for the template to save space
class WvBdbHashBase
{
public:
    // a very ugly way to avoid #including the db.h from here
    struct datum
    {
	void *dptr;
	size_t dsize;
    };
    
    WvBdbHashBase(WvStringParm dbfile);
    ~WvBdbHashBase();
    int count() const { return entries; }
    bool isempty() const { return !entries; }

    int add(const datum &key, const datum &data, bool replace);
    int remove(const datum &key);
    datum find(const datum &key);
    bool exists(const datum &key);
    void zap();
    
    class IterBase
    {
    public:
        IterBase(WvBdbHashBase &_bdbhash);
        ~IterBase();
        void rewind();
	void rewind(const datum &firstkey);
        void next();
        
    protected:
        WvBdbHashBase &bdbhash;
        datum curkey;
        datum curdata;
    };
private:
    friend class IterBase;
    int entries;
    struct __db *dbf;
};


/**
 * This hashtable is different from normal WvStreams hashtables in that it
 * stores the data on disk.
 * 
 * This affects memory management for objects stored in it.
 * 
 * For find and operator[], the returned object is only guaranteed to be
 * around until the next find/or next() for iterators. 
 */
template <class K, class D>
class WvBdbHash : public WvBdbHashBase
{
public:
    // this class is interchangeable with datum, but includes a WvDynBuf
    // object that datum.dptr points to.  So when this object goes away,
    // it frees its dptr automatically.
    template <typename T>
    class datumize : public datum
    {
	datumize(datumize &); // not defined
    public:
	WvDynBuf buf;
	
	datumize(const T &t)
	{
	    wv_serialize(buf, t);
	    dsize = buf.used();
	    dptr = (char *)buf.peek(0, buf.used());
	}
    };
    
    template <typename T>
    static T undatumize(datum &data)
    {
	WvConstInPlaceBuf buf(data.dptr, data.dsize);
	return wv_deserialize<T>(buf);
    }

protected:
    D *saveddata;
    
public:
    WvBdbHash(WvStringParm dbfile) : WvBdbHashBase(dbfile)
        { saveddata = NULL; }

    void add(const K &key, const D &data, bool replace = false)
    {
        int r = WvBdbHashBase::add(datumize<K>(key),
				    datumize<D>(data), replace);
        assert(!r && "Set the replace flag to replace existing elements.");
    }

    void remove(const K &key)
        { WvBdbHashBase::remove(datumize<K>(key)); }

    D &find(const K &key)
    {   
	if (saveddata)
	    delete saveddata;
	datum s = WvBdbHashBase::find(datumize<K>(key));
	saveddata = undatumize<D *>(s);
	return *saveddata;
    }

    D &operator[] (const K &key)
        { return find(key); }
        
    bool exists(const K &key)
        { return WvBdbHashBase::exists(datumize<K>(key)); }
    
    class Iter : public WvBdbHashBase::IterBase
    {
	K *k;
	D *d;
    public:
        Iter(WvBdbHash &_bdbhash) : IterBase(_bdbhash) 
	    { k = NULL; d = NULL; }
	~Iter()
	{
	    if (k) delete k;
	    if (d) delete d;
	}
	
	void rewind()
	    { IterBase::rewind(); }
	void rewind(const K &firstkey)
	    { IterBase::rewind(WvBdbHash::datumize<K>(firstkey)); }

        bool next()
        {
	    if (k) delete k;
	    if (d) delete d;
            IterBase::next();
	    if (curdata.dptr)
	    {
		k = undatumize<K *>(curkey);
		d = undatumize<D *>(curdata);
		return true;
	    }
	    else
	    {
		k = NULL;
		d = NULL;
		return false;
	    }
        }
        
	bool cur()
            { return curdata.dptr; }
	
	K &key() const
	    { return *k; }
	
        D *ptr() const
	    { return d; }
	WvIterStuff(D);
    };
};

#endif // __WVBDBHASH_H
