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
#include "wverror.h"

/**
 * This hashtable is different from normal WvStreams hashtables in that it
 * stores the data on disk.
 * 
 * This affects memory management for objects stored in it.
 * 
 * For find and operator[], the returned object is only guaranteed to be
 * around until the next find() (or next(), for iterators).  Remember that
 * you may not be the only person to do a next() or find() on this database.
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
    class Iter;
    
    // this class is interchangeable with datum, but includes a WvDynBuf
    // object that datum.dptr points to.  So when this object goes away,
    // it frees its dptr automatically.
    template <typename T>
    class datumize : public datum
    {
    private:
	void init(const T &t)
	{
	    wv_serialize(buf, t);
	    this->dsize = buf.used();
	    this->dptr = (char *)buf.peek(0, buf.used());
	}

    protected:
	datumize(datumize<T> &); // not defined

    public:
	WvDynBuf buf;

	datumize(const T &t)
            { init(t); }

	datumize(const T *t)
	{
	    if (t)
		init(*t);
	    else
	    {
		this->dsize = 0;
		this->dptr = 0;
	    }
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
    WvOnDiskHash(WvStringParm dbfile = WvString::null, bool persist = true) :
        Backend(dbfile, persist)
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
            typename MyType::template datumize<K> key(k);
            typename MyType::template datumize<D> data(d);

            BackendIterBase::rewind(typename MyType::template datumize<K>(
					firstkey), key, data);
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
            typename MyType::template datumize<K> key(k);
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
	{
	    BackendIterBase::xunlink(typename MyType::template datumize<K>(k));
	}

        void save()
	{
	    BackendIterBase::update(typename MyType::template datumize<K>(k),
				    typename MyType::template datumize<D>(d));
	}

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

/**
 * Base class for the template to save space
 */
class WvBdbHash : public WvErrorBase
{
    WvString dbfile;
    bool persist_dbfile;

public:
    struct datum
    {
        void *dptr;
        size_t dsize;
    };
    typedef datum Datum;

    WvBdbHash(WvStringParm _dbfile = WvString::null, bool persist = true);
    ~WvBdbHash();

    /**
     * Open a new db file.  This will instantly change the contents of the
     * db, and probably mess up all your iterators.  Best used just after
     * creation.
     *
     * if dbfile is NULL, bdb will create an "anonymous" database.  It'll
     * still take up disk space, but it disappears when closed.  If dbfile is
     * not NULL but persist_dbfile is false, the file will be truncated when
     * opened and deleted when closed.
     *
     * It is ok to use this if !isok - in fact, it's the expected way to reset
     * it.  It may fail and seterr itself, though, so don't get stuck in a
     * loop.
     */
    void opendb(WvStringParm _dbfile = WvString::null, bool persist = true);

    /**
     * Close the db file.  Makes isok return false, so you must call opendb()
     * before using it again.  The effect on open iterators is undefined.
     *
     * This can be called when !isok.  It will always set the error message to
     * "The db is closed" if it succeeds; if it sets it to anything else,
     * there was an error while flushing the db.
     */
    void closedb();
    
    void add(const datum &key, const datum &data, bool replace);
    void remove(const datum &key);
    datum find(const datum &key);
    bool exists(const datum &key);

    /**
     * Wipe the db.  Calling this while !isok is allowed, but not guaranteed
     * to fix it.
     */
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

// FIXME: the interface for this is quite backwards.  It should be delegating
// to WvQdbmHash, not inheriting from it!  In the meantime, see WvBdbHash for
// the API docs... 
class WvQdbmHash : public WvErrorBase
{
    bool persist_dbfile;

public:
    struct datum
    {
        const char *dptr;
        int dsize;
    };
    typedef datum Datum;

    WvQdbmHash(WvStringParm dbfile = WvString::null, bool persist = true);
    ~WvQdbmHash();
    
    void opendb(WvStringParm dbfile = WvString::null, bool persist = true);
    void closedb();
    
    void add(const Datum &key, const Datum &data, bool replace);
    void remove(const Datum &key);
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
    void *dbf;

    void dperr();
};
#endif // WITH_QDBM

#endif // __WVONDISKHASH_H
