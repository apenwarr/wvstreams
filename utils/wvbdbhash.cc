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
{
    dbf = dbopen(dbfile, O_CREAT|O_RDWR, 0777, DB_BTREE, NULL);
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
    if (!dbf) return -1;
    return dbf->put(dbf, (DBT *)&key, (DBT *)&data,
		    !replace ? R_NOOVERWRITE : 0);
}


int WvBdbHashBase::remove(const datum &key)
{
    if (!dbf) return -1;
    return dbf->del(dbf, (DBT *)&key, 0);
}


WvBdbHashBase::datum WvBdbHashBase::find(const datum &key)
{
    datum ret = {0, 0};
    if (!dbf) return ret;
    dbf->get(dbf, (DBT *)&key, (DBT *)&ret, 0);
    return ret;
}


bool WvBdbHashBase::exists(const datum &key)
{
    if (!dbf) return false; 
    datum ret = {0, 0};
    return !dbf->get(dbf, (DBT *)&key, (DBT *)&ret, 0);
}


void WvBdbHashBase::zap()
{
    datum key, value;
    if (!dbf) return;
    while (!dbf->seq(dbf, (DBT *)&key, (DBT *)&value, R_FIRST))
	dbf->del(dbf, (DBT *)&key, 0);
}


WvBdbHashBase::IterBase::IterBase(WvBdbHashBase &_bdbhash)
    : bdbhash(_bdbhash)
{
    curkey.dptr = curdata.dptr = NULL;
}


WvBdbHashBase::IterBase::~IterBase()
{
}


void WvBdbHashBase::IterBase::rewind()
{
    curkey.dptr = NULL;
}


void WvBdbHashBase::IterBase::next()
{
    if (bdbhash.dbf->seq(bdbhash.dbf, (DBT *)&curkey, (DBT *)&curdata,
			 curkey.dptr ? R_NEXT : R_FIRST))
	curkey.dptr = curdata.dptr = NULL;
}


