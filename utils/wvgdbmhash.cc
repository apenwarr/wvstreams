/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2003 Net Integration Technologies, Inc.
 *
 * A hash table container backed by a gdbm database.  See wvgdbmhash.h.
 */
#include "wvautoconf.h"

#if HAVE_LIBGDBM

#include "wvgdbmhash.h"


WvGdbmHashBase::WvGdbmHashBase(WvStringParm dbfile)
{
    dbf = gdbm_open((char *)dbfile.cstr(), 0, GDBM_WRCREAT, 0600, 0);
    if (!dbf)
        fprintf (stderr, "Could not open database '%s': %s\n",
                dbfile.cstr(), gdbm_strerror(gdbm_errno));
}


WvGdbmHashBase::~WvGdbmHashBase()
{
    gdbm_close(dbf);
}


int WvGdbmHashBase::add(const datum &key, const datum &data, bool replace)
{
    return gdbm_store(dbf, key, data,
            replace ? GDBM_REPLACE : GDBM_INSERT);
}


int WvGdbmHashBase::remove(const datum &key)
{
    return gdbm_delete(dbf, key);
}


datum WvGdbmHashBase::find(const datum &key)
{   
    return gdbm_fetch(dbf, key);
}


bool WvGdbmHashBase::exists(const datum &key)
{
    return gdbm_exists(dbf, key);
}


WvGdbmHashBase::IterBase::IterBase(WvGdbmHashBase &_gdbmhash)
    : gdbmhash(_gdbmhash)
{
    curkey.dptr = nextkey.dptr = curdata.dptr = NULL;
}


WvGdbmHashBase::IterBase::~IterBase()
{
    free(curkey.dptr);
    free(nextkey.dptr);
    free(curdata.dptr);
}


void WvGdbmHashBase::IterBase::rewind()
{
    free(curkey.dptr);
    free(nextkey.dptr);
    curkey.dptr = NULL;
    nextkey = gdbm_firstkey(gdbmhash.dbf);
}


void WvGdbmHashBase::IterBase::next()
{
    free(curkey.dptr);
    free(curdata.dptr);
    curkey = nextkey;
    nextkey = gdbm_nextkey(gdbmhash.dbf, curkey);
    curdata = gdbm_fetch(gdbmhash.dbf, curkey);
}

#endif
