/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2003 Net Integration Technologies, Inc.
 *
 * A hash table container backed by a Berkeley DB (libdb) database.
 * See wvbdbhash.h.
 */

#include "wvautoconf.h"

#ifdef WITH_BDB

#include "wvbdbhash.h"
#include <fcntl.h>
#include <errno.h>

#ifdef HAVE_DB_H
#include <db.h>
#else
#ifdef HAVE_DB_185_H
#include <db_185.h>
#endif
#endif


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


WvBdbHashBase::WvBdbHashBase(WvStringParm dbfile) :
    dbf(NULL)
{
    opendb(dbfile);
}


WvBdbHashBase::~WvBdbHashBase()
{
    if (dbf)
	dbf->close(dbf);
}


void WvBdbHashBase::opendb(WvStringParm dbfile)
{
    if (dbf) dbf->close(dbf);
    
    BTREEINFO info;
    memset(&info, 0, sizeof(info));
    info.compare = comparefunc;
    dbf = dbopen(!!dbfile ? dbfile.cstr() : NULL, O_CREAT|O_RDWR, 0666,
            DB_BTREE, &info);
    if (!dbf)
        fprintf(stderr, "Could not open database '%s': %s\n",
                dbfile.cstr(), strerror(errno));
}


int WvBdbHashBase::add(const datum &key, const datum &data, bool replace)
{
    assert(isok());
    return dbf->put(dbf, (DBT *)&key, (DBT *)&data,
		    !replace ? R_NOOVERWRITE : 0);
}


int WvBdbHashBase::remove(const datum &key)
{
    assert(isok());
    return dbf->del(dbf, (DBT *)&key, 0);
}


WvBdbHashBase::datum WvBdbHashBase::find(const datum &key)
{
    assert(isok());
    datum ret = {0, 0};
    dbf->get(dbf, (DBT *)&key, (DBT *)&ret, 0);
    return ret;
}


bool WvBdbHashBase::exists(const datum &key)
{
    assert(isok());
    datum ret = {0, 0};
    return !dbf->get(dbf, (DBT *)&key, (DBT *)&ret, 0);
}


void WvBdbHashBase::zap()
{
    assert(isok());
    datum key, value;
    while (!dbf->seq(dbf, (DBT *)&key, (DBT *)&value, R_FIRST))
	dbf->del(dbf, (DBT *)&key, 0);
}


WvBdbHashBase::IterBase::IterBase(WvBdbHashBase &_bdbhash)
    : bdbhash(_bdbhash)
{
    rewindto.dsize = 0;
    rewindto.dptr = NULL;
}


WvBdbHashBase::IterBase::~IterBase()
{
    free(rewindto.dptr);
}


void WvBdbHashBase::IterBase::rewind()
{
    free(rewindto.dptr);
    rewindto.dptr = NULL;
}


void WvBdbHashBase::IterBase::rewind(const datum &firstkey, datum &curkey,
        datum &curdata)
{
    assert(bdbhash.isok());

    // save the firstkey and clear the current one
    free(rewindto.dptr);
    rewindto.dsize = firstkey.dsize;
    rewindto.dptr = malloc(rewindto.dsize);
    memcpy(rewindto.dptr, firstkey.dptr, rewindto.dsize);
    curkey.dptr = curdata.dptr = NULL;
}


void WvBdbHashBase::IterBase::next(datum &curkey, datum &curdata)
{
    assert(bdbhash.isok());

    // check if this is the first next() after a rewind()
    bool first = !curkey.dptr;
    datum wanted = { 0, 0 };
    if (first) {
        if (rewindto.dptr) {
            curkey = rewindto;
            first = false;
        }
    } else {
        wanted.dsize = curkey.dsize;
        wanted.dptr = malloc(wanted.dsize);
        memcpy(wanted.dptr, curkey.dptr, wanted.dsize);
    }

    // always seek for the saved cursor we were just passed, to work around
    // bugs in libdb1's seq with btrees.  (As a bonus, this gives us multiple
    // iterators for free!)
    if (bdbhash.dbf->seq(bdbhash.dbf, (DBT *)&curkey, (DBT *)&curdata,
                first ? R_FIRST : R_CURSOR))
    {
        // current key gone, and none higher left: done
	curkey.dptr = curdata.dptr = NULL;
    }
    else if (wanted.dptr && !comparefunc((DBT *) &curkey, (DBT *) &wanted))
    {
        // found the exact key requested, so move forward one
        if (bdbhash.dbf->seq(bdbhash.dbf, (DBT *)&curkey, (DBT *)&curdata,
                    R_NEXT))
        {
            // nothing left?  Fine, we're done
            curkey.dptr = curdata.dptr = NULL;
        }
    }

    // otherwise, curkey is now sitting on the key after the requested one (or
    // the very first key), as expected.  (Also, if rewindto is set it should
    // be either filled in with the matching btree data or cleared.)
    assert(!rewindto.dptr || curkey.dptr != rewindto.dptr);
    free(wanted.dptr);
}


void WvBdbHashBase::IterBase::xunlink(const datum &curkey)
{
    bdbhash.remove(curkey);
}


void WvBdbHashBase::IterBase::update(const datum &curkey, const datum &data)
{
    int r = bdbhash.add(curkey, data, true);
    assert(!r && "Weird: database add failed during save?");
}

#endif /* WITH_BDB */
