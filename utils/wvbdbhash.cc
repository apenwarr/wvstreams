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


WvBdbHash::WvBdbHash(WvStringParm dbfile) :
    dbf(NULL)
{
    opendb(dbfile);
}


WvBdbHash::~WvBdbHash()
{
    if (dbf)
	dbf->close(dbf);
}


void WvBdbHash::opendb(WvStringParm dbfile)
{
    if (dbf)
        dbf->close(dbf);
    
    BTREEINFO info;
    memset(&info, 0, sizeof(info));
    info.compare = comparefunc;
    dbf = dbopen(!!dbfile ? dbfile.cstr() : NULL, O_CREAT|O_RDWR, 0666,
            DB_BTREE, &info);
    if (!dbf)
        fprintf(stderr, "Could not open database '%s': %s\n",
                dbfile.cstr(), strerror(errno));
}


void WvBdbHash::add(const datum &key, const datum &data, bool replace)
{
    assert(isok());
    int r = dbf->put(dbf, (DBT *)&key, (DBT *)&data,
		    !replace ? R_NOOVERWRITE : 0);
    if (r)
        fprintf(stderr, "Error: %s\n", strerror(errno));
}


int WvBdbHash::remove(const datum &key)
{
    assert(isok());
    datum newkey, data;
    newkey = key;
    
    int ret = dbf->seq(dbf, (DBT *)&newkey, (DBT *)&data, R_CURSOR);
    if (!ret)
	return dbf->del(dbf, (DBT *)&newkey, R_CURSOR);
    else
	return ret;
}


WvBdbHash::datum WvBdbHash::find(const datum &key)
{
    assert(isok());
    datum ret = {0, 0};
    dbf->get(dbf, (DBT *)&key, (DBT *)&ret, 0);
    return ret;
}


bool WvBdbHash::exists(const datum &key)
{
    assert(isok());
    datum ret = {0, 0};
    return !dbf->get(dbf, (DBT *)&key, (DBT *)&ret, 0);
}


void WvBdbHash::zap()
{
    assert(isok());
    datum key, value;
    while (!dbf->seq(dbf, (DBT *)&key, (DBT *)&value, R_FIRST))
	dbf->del(dbf, (DBT *)&key, R_CURSOR);
}

void WvBdbHash::IterBase::next(datum &curkey, datum &curdata)
{
    assert(parent.isok());

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
    if (parent.dbf->seq(parent.dbf, (DBT *)&curkey, (DBT *)&curdata,
                first ? R_FIRST : R_CURSOR))
    {
        // current key gone, and none higher left: done
	curkey.dptr = curdata.dptr = NULL;
    }
    else if (!first)
    {
	while (comparefunc((DBT *)&wanted, (DBT *)&curkey) >= 0)
	{
	    // found the exact key or earlier than requested: move forward one
	    // (yes, libbdb1 can return a key less than requested, despite
	    // the documentation's claims!)
	    // This algorithm definitely makes it so inserting the same key
	    // more than once doesn't work at all.
	    if (parent.dbf->seq(parent.dbf, (DBT *)&curkey, (DBT *)&curdata,
				 R_NEXT))
	    {
		// nothing left?  Fine, we're done
		curkey.dptr = curdata.dptr = NULL;
		break;
	    }
	}
    }

    // otherwise, curkey is now sitting on the key after the requested one (or
    // the very first key), as expected.  (Also, if rewindto is set it should
    // be either filled in with the matching btree data or cleared.)
    assert(!rewindto.dptr || curkey.dptr != rewindto.dptr);
    free(wanted.dptr);
}

#endif /* WITH_BDB */
