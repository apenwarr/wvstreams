/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2003 Net Integration Technologies, Inc.
 *
 * A hash table container backed by a Berkeley DB (libdb) database.
 * See wvondiskhash.h.
 */
#include "wvautoconf.h"

#ifdef WITH_BDB

#include "wvondiskhash.h"
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#ifdef HAVE_DB_H
#include <db.h>
#else
#ifdef HAVE_DB_185_H
#include <db_185.h>
#endif
#endif

#include "wvlog.h"

int comparefunc(const DBT *a, const DBT *b)
{
    if (a == NULL && b == NULL) return 0;
    if (a == NULL) return 1;
    if (b == NULL) return -1;
    
    size_t minlen = (a->size > b->size) ? b->size : a->size;
    int ret = memcmp(a->data, b->data, minlen);
    if (ret != 0) return ret;
    if (a->size > b->size) return 1;
    if (a->size < b->size) return -1;
    return 0;
}


WvBdbHash::WvBdbHash(WvStringParm _dbfile, bool _persist)
{
    dbf = NULL;
    opendb(_dbfile, _persist);
}


WvBdbHash::~WvBdbHash()
{
    closedb();
}


void WvBdbHash::opendb(WvStringParm _dbfile, bool _persist)
{
    closedb();

    noerr();
    
    dbfile = _dbfile;
    persist_dbfile = _persist;

    BTREEINFO info;
    memset(&info, 0, sizeof(info));
    info.compare = comparefunc;

    int mode = O_CREAT | O_RDWR;
    if (!persist_dbfile) mode |= O_TRUNC;
    
    dbf = dbopen(!!dbfile ? dbfile.cstr() : NULL,
            mode, 0666, DB_BTREE, &info);
    if (dbf == NULL) seterr(errno);
}


void WvBdbHash::closedb()
{
    if (dbf)
    {
        if (dbf->close(dbf) != 0) seterr(errno);
        if (!persist_dbfile && !!dbfile)
            ::unlink(dbfile);
        dbf = NULL;
    }
    seterr("The db is closed.");    // this won't overwrite an earlier err
}


void WvBdbHash::add(const datum &key, const datum &data, bool replace)
{
    if (!isok()) return;
    int r = dbf->put(dbf, (DBT *)&key, (DBT *)&data,
		    !replace ? R_NOOVERWRITE : 0);
    if (r == 1)
        seterr("Must set the replace flag to replace existing elements.");
    else if (r != 0)
        seterr(errno);
}


void WvBdbHash::remove(const datum &key)
{
    if (!isok()) return;
    
    datum newkey, data;
    newkey = key;
    
    int ret = dbf->seq(dbf, (DBT *)&newkey, (DBT *)&data, R_CURSOR);
    if (!ret)
    {
	ret = dbf->del(dbf, (DBT *)&newkey, R_CURSOR);
    }
    
    if (ret == 1) seterr("Strange: seq found a key that del didn't recognize");
    else if (ret) seterr(errno);
}


WvBdbHash::datum WvBdbHash::find(const datum &key)
{
    datum ret = {0, 0};
    if (!isok()) return ret;

    int r = dbf->get(dbf, (DBT *)&key, (DBT *)&ret, 0);
    if (r == 1)
    {
        // not found - make sure we return an empty datum
        ret.dptr = NULL;
    }
    else if (r != 0)
    {
        ret.dptr = NULL;
        seterr(errno);
    }
    return ret;
}


bool WvBdbHash::exists(const datum &key)
{
    if (!isok()) return false;

    datum ret = {0, 0};
    int r = dbf->get(dbf, (DBT *)&key, (DBT *)&ret, 0);

    // return true on success
    if (r == 0) return true;

    // return false on 1 (not found) or -1 (error)
    if (r != 1) seterr(errno);
    return false;
}


void WvBdbHash::zap()
{
    if (!dbfile)
    {
        // we're broken - nothing we can do
        if (!isok())
        {
            closedb();
            return;
        }

        // super-slow version
        datum key, value;
        int r;
        while ((r = dbf->seq(dbf, (DBT *)&key, (DBT *)&value, R_FIRST)) == 0)
        {
            int r2 = dbf->del(dbf, (DBT *)&key, R_CURSOR);
            if (r2 == 1) seterr("Strange: seq found a key that del didn't recognize");
            else if (r2 != 0) seterr(errno);
        }
        if (r != 1) seterr(errno);  // 1 = not found
    }
    else // delete the database file and reopen - much quicker!
    {
        if (dbf)
        {
            dbf->close(dbf);    // don't care if it fails
            dbf = NULL;
        }
        int fd = open(dbfile, O_RDWR | O_TRUNC);
        if (fd >= 0) ::close(fd);
        opendb(dbfile, persist_dbfile);
    }
}


void WvBdbHash::IterBase::next(datum &curkey, datum &curdata)
{
    if (!parent.isok()) return;

    // check if this is the first next() after a rewind()
    bool first = !curkey.dptr;
    datum wanted = { 0, 0 };
    if (first)
    {
        if (rewindto.dptr)
	{
            curkey = rewindto;
            first = false;
        }
    }
    else
    {
        wanted.dsize = curkey.dsize;
        wanted.dptr = malloc(wanted.dsize);
        memcpy(wanted.dptr, curkey.dptr, wanted.dsize);
    }

    // always seek for the saved cursor we were just passed, to work around
    // bugs in libdb1's seq with btrees.  (As a bonus, this gives us multiple
    // iterators for free!)
    int r = parent.dbf->seq(parent.dbf, (DBT *)&curkey, (DBT *)&curdata,
                first ? R_FIRST : R_CURSOR);
    
    if (r == 1)
    {
        // current key gone, and none higher left: done
	curkey.dptr = curdata.dptr = NULL;
    }
    else if (r != 0)
        parent.seterr(errno);

    else if (!first)
    {
	while (comparefunc((DBT *)&wanted, (DBT *)&curkey) >= 0)
	{
	    // found the exact key or earlier than requested: move forward one
	    // (yes, libbdb1 can return a key less than requested, despite
	    // the documentation's claims!)
	    // This algorithm definitely makes it so inserting the same key
	    // more than once doesn't work at all.
	    r = parent.dbf->seq(parent.dbf, (DBT *)&curkey, (DBT *)&curdata,
				 R_NEXT);

            if (r == 1)
	    {
		// nothing left?  Fine, we're done
		curkey.dptr = curdata.dptr = NULL;
		break;
	    }
            else if (r != 0)
                parent.seterr(errno);
	}
    }

    // otherwise, curkey is now sitting on the key after the requested one (or
    // the very first key), as expected.  (Also, if rewindto is set it should
    // be either filled in with the matching btree data or cleared.)  Unless,
    // of course, the whole db is borked.
    assert(!parent.isok() || !rewindto.dptr || curkey.dptr != rewindto.dptr);
    free(wanted.dptr);
}

#endif /* WITH_BDB */
