#include "wvgdbmhash.h"

#ifdef WITH_GDBM

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

int WvGdbmHashBase::add(datum key, datum data, bool replace)
{
    return gdbm_store(dbf, key, data,
            replace ? GDBM_REPLACE : GDBM_INSERT);
}

int WvGdbmHashBase::remove(datum key)
{
    return gdbm_delete(dbf, key);
}

datum WvGdbmHashBase::find(datum key)
{   
    return gdbm_fetch(dbf, key);
}

bool WvGdbmHashBase::exists(datum key)
{
    return gdbm_exists(dbf, key);
}

WvGdbmHashBase::IterBase::IterBase(WvGdbmHashBase &_gdbmhash)
    : gdbmhash(_gdbmhash)
{
    nextkey.dptr = curdata.dptr = NULL;
}

WvGdbmHashBase::IterBase::~IterBase()
{
    free(nextkey.dptr);
    free(curdata.dptr);
}

void WvGdbmHashBase::IterBase::rewind()
{
    free (nextkey.dptr);
    nextkey = gdbm_firstkey(gdbmhash.dbf);
}

void WvGdbmHashBase::IterBase::next()
{
    free (curdata.dptr);
    curdata = gdbm_fetch(gdbmhash.dbf, nextkey);
    free (nextkey.dptr);
    nextkey = gdbm_nextkey(gdbmhash.dbf, nextkey);
}

#endif
