/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2003 Net Integration Technologies, Inc.
 *
 * A hash table container backed by a qdbm database.
 */

#include "wvautoconf.h"

#ifdef WITH_QDBM

#include "wvondiskhash.h"

extern "C" {
#include <depot.h>
#include <cabin.h>
#include <villa.h>
#include <stdlib.h>
}

WvQdbmHash::WvQdbmHash(WvStringParm dbfile) : dbf(0)
{
    saveddata.dptr = 0;
    opendb(dbfile);
}


WvQdbmHash::~WvQdbmHash()
{
    free((void*)saveddata.dptr);
    closedb();
}


void WvQdbmHash::closedb()
{
    if (dbf)
        vlclose((VILLA*)dbf);
    dbf = 0;
}


void WvQdbmHash::opendb(WvStringParm dbfile)
{
    // VL_CMPLEX = lexigraphic comparison
    dbf = vlopen(dbfile.cstr(), VL_OWRITER|VL_OCREAT, VL_CMPLEX);
    if (!dbf)
        fprintf (stderr, "Could not open database '%s': %s\n",
                dbfile.cstr(), dperrmsg(dpecode));
}


void WvQdbmHash::add(const datum &key, const datum &data, bool replace)
{
    int r = vlput((VILLA*)dbf, key.dptr, key.dsize, data.dptr, data.dsize,
            replace ? VL_DOVER : VL_DKEEP);

    if (!r)
        fprintf(stderr, "Error: %s\n", dperrmsg(dpecode));
}


int WvQdbmHash::remove(const datum &key)
{
    return vlout((VILLA*)dbf, key.dptr, key.dsize);
}


WvQdbmHash::Datum WvQdbmHash::find(const datum &key)
{   
    free((void*)saveddata.dptr);
    saveddata.dptr = vlget((VILLA*)dbf, key.dptr, key.dsize,
            &saveddata.dsize);
    return saveddata;
}


bool WvQdbmHash::exists(const datum &key)
{
    return vlvnum((VILLA*)dbf, key.dptr, key.dsize);
}


void WvQdbmHash::zap()
{
    if (!dbf)
        return;

    char *name = vlname((VILLA*)dbf);
    closedb();
    vlremove(name);
    opendb(name);
    free(name);
}


void WvQdbmHash::IterBase::next(datum &curkey, datum &curdata)
{
    // Dude, this function is a mess!
    if (!parent.isok())
        return;

    VILLA *dbf = (VILLA*)parent.dbf;
    Datum wanted = { 0, 0 };

    if (curkey.dptr)
    {
        // If we are given the current key, seek to it
        // but make a backup of the key first to check if the seek was
        // successful
        wanted.dsize = curkey.dsize;
        wanted.dptr = (typeof(wanted.dptr))malloc(wanted.dsize);
        memcpy((void*)wanted.dptr, curkey.dptr, wanted.dsize);
        vlcurjump(dbf, curkey.dptr, curkey.dsize, VL_JFORWARD);
    }
    else if (rewindto.dptr)
        vlcurjump(dbf, rewindto.dptr, rewindto.dsize, VL_JFORWARD);
    else
        vlcurfirst(dbf);

    // obtain the current key
    curkey.dptr = vlcurkey((VILLA*)parent.dbf, &curkey.dsize);

    // if current key is the same as the backup....
    if (wanted.dptr && wanted.dsize == curkey.dsize
            && !memcmp(wanted.dptr, curkey.dptr, wanted.dsize))
    {
        // the seek was successful. so go forward one more.
        vlcurnext(dbf);
        // and re-obtain the key
        curkey.dptr = vlcurkey((VILLA*)parent.dbf, &curkey.dsize);
    }

    // if we managed to get a valid key, fetch the data for it
    if (curkey.dptr)
        curdata = parent.find(curkey);

    free((void*)wanted.dptr);
}

#endif
