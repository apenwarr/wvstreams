/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2003 Net Integration Technologies, Inc.
 *
 * A hash table container backed by a Berkeley DB (libdb) database.
 * See wvbdbhash.h.
 */
#include "wvbdbhash.h"
#include <fcntl.h>
#include <errno.h>

#include <db_185.h>
//#include </tmp/db1-compat-2.1.3/db.h>


WvBdbHashBase::WvBdbHashBase(WvStringParm dbfile)
    : itercount(0)
{
    dbf = dbopen(dbfile, O_CREAT|O_RDWR, 0666, DB_BTREE, NULL);
    if (!dbf)
        fprintf(stderr, "Could not open database '%s': %s\n",
                dbfile.cstr(), strerror(errno));
}


WvBdbHashBase::~WvBdbHashBase()
{
    if (dbf)
	dbf->close(dbf);
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
    curkey.dptr = curdata.dptr = NULL;
    assert(++bdbhash.itercount == 1);
}


WvBdbHashBase::IterBase::~IterBase()
{
    bdbhash.itercount--;
}


void WvBdbHashBase::IterBase::rewind()
{
    curkey.dptr = NULL;
    empty = false;
}


void WvBdbHashBase::IterBase::rewind(const datum &firstkey)
{
    assert(bdbhash.isok());
    curkey.dptr = NULL;
    empty = false;
    
    curkey = firstkey;
    if (bdbhash.dbf->seq(bdbhash.dbf, (DBT *)&curkey, (DBT *)&curdata,
			 R_CURSOR))
    {
	// no key >= requested one?  empty set.
	curkey.dptr = NULL;
	empty = true;
    }
    else if (bdbhash.dbf->seq(bdbhash.dbf, (DBT *)&curkey, (DBT *)&curdata,
			      R_PREV))
    {
	// no key < requested one?  start at the beginning.
	curkey.dptr = NULL;
    }
    
    // otherwise, curkey is now the key right before the requested one.
    // That makes next() go to the first requested key, following standard
    // WvUtils semantics.
}


void WvBdbHashBase::IterBase::next()
{
    assert(bdbhash.isok());
    if (bdbhash.dbf->seq(bdbhash.dbf, (DBT *)&curkey, (DBT *)&curdata,
			 curkey.dptr ? R_NEXT : R_FIRST))
	curkey.dptr = curdata.dptr = NULL;
}


