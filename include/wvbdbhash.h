/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2003 Net Integration Technologies, Inc.
 *
 * A hash table container backed by a Berkeley DB (libdb) database.
 * Intended to work with versions as old as libdb1.
 */
#ifndef __WVBDBHASH_H
#define __WVBDBHASH_H

#include "wvautoconf.h"

#ifndef WITH_BDB
# error "Sorry, no Berkeley DB support in WvStreams!"
#endif

#include "wvhashtable.h"
#include "wvserialize.h"

// Base class for the template to save space
class WvBdbHashBase
{
    WvString dbfile;
    bool persist_dbfile;

public:
    // a very ugly way to avoid #including the db.h from here.  This has
    // to be binary-compatible with the DBT structure.
    struct datum
    {
	void *dptr;
	size_t dsize;
    };
    
    WvBdbHashBase(WvStringParm _dbfile, bool persist_dbfile = true);
    ~WvBdbHashBase();

    /**
     * Open a new db file.  This will instantly change the contents of the
     * db, and probably mess up all your iterators.  Best used just after
     * creation.
     * 
     * if dbfile is NULL, bdb will create an "anonymous" database.  It'll
     * still take up disk space, but it disappears when closed.  If dbfile is
     * not NULL but persist_dbfile is false, the file will be truncated when
     * opened and deleted when closed.
     */
    void opendb(WvStringParm _dbfile, bool persist_dbfile = true);

    /**
     * Close the db file.  Makes isok return false, so you must call opendb()
     * before using it again.  The effect on open iterators is undefined.
     */
    void closedb();

    bool isok() const
        { return dbf; }

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
	void rewind(const datum &firstkey, datum &key, datum &data);
        void next(datum &key, datum &data);
        void xunlink(const datum &key);
        void update(const datum &key, const datum &data);
        
    protected:
        WvBdbHashBase &bdbhash;
        datum rewindto;
    };
   
private:
    friend class IterBase;
    struct __db *dbf;
};


/**
 * This hashtable is different from normal WvStreams hashtables in that it
 * stores the data on disk.
 * 
 * This affects memory management for objects stored in it.
 * 
 * For find and operator[], the returned object is only guaranteed to be
 * around until the next find() (or next(), for iterators).  Remember that
 * you may not be the only person to do a next() or find() on this database.
 *
 * You may only have one iterator at a time for a given WvBdbHash (for the
 * moment at least).  This is due to the strange way in which the database
 * handles iteration (with its own internal cursor).  Note that first()
 * and count() also use iterators!
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

        void init(const T &t)
        {
	    wv_serialize(buf, t);
	    dsize = buf.used();
	    dptr = (char *)buf.peek(0, buf.used());
        }

    public:
	WvDynBuf buf;
	
	datumize(const T &t)
            { init(t); }

	datumize(const T *t)
        {
            if (t) { init(*t); }
            else { dsize = 0; dptr = 0; }
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
    WvBdbHash(WvStringParm dbfile = WvString::null, bool persist = true) :
        WvBdbHashBase(dbfile, persist) { saveddata = NULL; }

    void add(const K &key, const D &data, bool replace = false)
    {
        int r = WvBdbHashBase::add(datumize<K>(key),
				    datumize<D>(data), replace);
        assert((!r || replace)
	       && "Set the replace flag to replace existing elements.");
	assert(!r && "Weird: database add failed?");
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

    int count()
    {
	int res = 0;
	Iter i(*this);
	for (i.rewind(); i.next(); )
	    res++;
	return res;
    }

    bool isempty()
    {
        Iter i(*this);
        i.rewind();
        return !i.next();
    }
 
    D &first()
    {
	Iter i(*this);
	i.rewind(); i.next();
	return i();
    }

    class Iter : public WvBdbHashBase::IterBase
    {
	K *k;
	D *d;

    public:
        Iter(WvBdbHash &_bdbhash) : IterBase(_bdbhash) 
	    { k = NULL; d = NULL; }
	~Iter()
	{
	    delete k;
	    delete d;
	}
	
	void rewind()
        {
            IterBase::rewind();
            delete k; k = NULL;
            delete d; d = NULL;
        }

	void rewind(const K &firstkey)
        {
            WvBdbHash::datumize<K> key(k);
            WvBdbHash::datumize<D> data(d);

            IterBase::rewind(WvBdbHash::datumize<K>(firstkey), key, data);
            delete k;
            delete d;
            if (data.dptr)
            {
                k = undatumize<K *>(key);
                d = undatumize<D *>(data);
            }
            else
            {
                k = NULL;
                d = NULL;
            }
        }

        bool next()
        {
            WvBdbHash::datumize<K> key(k);
            datum data = { 0, 0 };
            IterBase::next(key, data);
            delete k;
            delete d;
            if (data.dptr)
            {
                k = undatumize<K *>(key);
                d = undatumize<D *>(data);
                return true;
            }
            k = NULL;
            d = NULL;
            return false;
        }
    
        void unlink()
            { xunlink(); next(); }
        
        void xunlink()
            { IterBase::xunlink(WvBdbHash::datumize<K>(k)); }

        void save()
            { IterBase::update(WvBdbHash::datumize<K>(k),
                    WvBdbHash::datumize<D>(d)); }

	bool cur()
            { return d; }
	
	K &key() const
	    { assert(k); return *k; }
	
        D *ptr() const
	    { return d; }

	WvIterStuff(D);

    };

};

#endif // __WVBDBHASH_H
