/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2003 Net Integration Technologies, Inc.
 *
 * A hash table container backed by an on disk hash table,
 *   such as Berkeley DB (libdb) database, or GDBM
 */

#ifndef __WVONDISKHASH_H
#define __WVONDISKHASH_H

#include "wvautoconf.h"
#include "wvhashtable.h"
#include "wvserialize.h"

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
 * You may only have one iterator at a time for a given hash table (for the
 * moment at least).  This is due to the strange way in which the database
 * handles iteration (with its own internal cursor).  Note that first()
 * and count() also use iterators!
 */

// default to QDBM if available because it's faster
#ifdef WITH_QDBM
class WvQdbmHash;
typedef WvQdbmHash DefaultHash;
#else
# ifdef WITH_BDB
class WvBdbHash;
typedef WvBdbHash DefaultHash;
# else
#  error No supported database found!
# endif
#endif

template <class K, class D, class Backend = DefaultHash>
class WvOnDiskHash : public Backend
{
    typedef WvOnDiskHash<K, D, Backend> MyType;
    typedef typename Backend::IterBase BackendIterBase;
    typedef typename Backend::Datum datum;

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
    WvOnDiskHash(WvStringParm dbfile = WvString::null) : Backend(dbfile)
        { saveddata = NULL; }

    ~WvOnDiskHash()
        { delete saveddata; }

    void add(const K &key, const D &data, bool replace = false)
        { Backend::add(datumize<K>(key), datumize<D>(data), replace); }

    void remove(const K &key)
        { Backend::remove(datumize<K>(key)); }

    D &find(const K &key)
    {   
        delete saveddata;
	datum s = Backend::find(datumize<K>(key));
	saveddata = undatumize<D *>(s);
	return *saveddata;
    }

    D &operator[] (const K &key)
        { return find(key); }
        
    bool exists(const K &key)
        { return Backend::exists(datumize<K>(key)); }

    size_t count()
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

    class Iter : public BackendIterBase
    {
	K *k;
	D *d;

    public:
        Iter(MyType &hash) : BackendIterBase(hash)
	    { k = NULL; d = NULL; }
	~Iter()
	{
	    delete k;
	    delete d;
	}
	
	void rewind()
        {
            BackendIterBase::rewind();
            delete k; k = NULL;
            delete d; d = NULL;
        }

	void rewind(const K &firstkey)
        {
            MyType::datumize<K> key(k);
            MyType::datumize<D> data(d);

            BackendIterBase::rewind(MyType::datumize<K>(firstkey), key, data);
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
            MyType::datumize<K> key(k);
            datum data = { 0, 0 };
            BackendIterBase::next(key, data);
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
            { BackendIterBase::xunlink(MyType::datumize<K>(k)); }

        void save()
            { BackendIterBase::update(MyType::datumize<K>(k),
                    MyType::datumize<D>(d)); }

	bool cur()
            { return d; }
	
	K &key() const
	    { assert(k); return *k; }
	
        D *ptr() const
	    { return d; }

	WvIterStuff(D);
    };

};

template <class Parent, class Datum>
class WvOnDiskHashIterBase
{
public:
    WvOnDiskHashIterBase(Parent &_parent) : parent(_parent)
    {
        rewindto.dsize = 0;
        rewindto.dptr = NULL;
    }
    ~WvOnDiskHashIterBase() { free((void *)rewindto.dptr); }
    void rewind()
    {
        free((void*)rewindto.dptr);
        rewindto.dptr = NULL;
    }
    void rewind(const Datum &firstkey, Datum &curkey, Datum &curdata)
    {
        assert(parent.isok());
        // save the firstkey and clear the current one
        free((void*)rewindto.dptr);
        rewindto.dsize = firstkey.dsize;
        rewindto.dptr = (typeof(rewindto.dptr))malloc(rewindto.dsize);
        memcpy((void*)rewindto.dptr, firstkey.dptr, rewindto.dsize);
        curkey.dptr = curdata.dptr = NULL;
    }
    void xunlink(const Datum &curkey) { parent.remove(curkey); }
    void update(const Datum &curkey, const Datum &data)
        { parent.add(curkey, data, true); }

protected:
    Datum rewindto;
    Parent &parent;
};

#ifdef WITH_BDB

// Base class for the template to save space
class WvBdbHash
{
public:
    struct datum
    {
        void *dptr;
        size_t dsize;
    };
    typedef datum Datum;

    WvBdbHash(WvStringParm dbfile = WvString::null);
    ~WvBdbHash();

    /**
     * Open a new db file.  This will instantly change the contents of the
     * db, and probably mess up all your iterators.  Best used just after
     * creation.
     */
    void opendb(WvStringParm dbfile = WvString::null);
    
    bool isok() const
        { return dbf; }

    void add(const datum &key, const datum &data, bool replace);
    int remove(const datum &key);
    datum find(const datum &key);
    bool exists(const datum &key);
    void zap();
    
    class IterBase : public WvOnDiskHashIterBase<WvBdbHash, Datum>
    {
    public:
        IterBase(WvBdbHash &bdbhash)
            : WvOnDiskHashIterBase<WvBdbHash, Datum>(bdbhash) {};

        void next(datum &key, datum &data);
    };
   
private:
    friend class IterBase;
    struct __db *dbf;
};
#endif // WITH_BDB

#ifdef WITH_QDBM

class WvQdbmHash
{
public:
    struct datum
    {
        const char *dptr;
        int dsize;
    };
    typedef datum Datum;

    WvQdbmHash(WvStringParm dbfile);
    ~WvQdbmHash();
    
    void opendb(WvStringParm dbfile);
    
    bool isok() const
        { return dbf; }

    void add(const Datum &key, const Datum &data, bool replace);
    int remove(const Datum &key);
    Datum find(const Datum &key);
    bool exists(const Datum &key);
    void zap();
    
    class IterBase : public WvOnDiskHashIterBase<WvQdbmHash, Datum>
    {
    public:
        IterBase(WvQdbmHash &qdbmhash)
            : WvOnDiskHashIterBase<WvQdbmHash, Datum>(qdbmhash) {};

        void next(Datum &key, Datum &data);
    };
private:
    Datum saveddata;
    friend class IterBase;
    void closedb();
    void *dbf;
};
#endif // WITH_QDBM

#endif // __WVONDISKHASH_H
