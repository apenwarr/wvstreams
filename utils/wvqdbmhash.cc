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
#include <unistd.h>
}


/** NB: be careful!  The return values of most qdbm functions are the
 * opposite of bdb.  bdb uses 0 for success, -1 for error; qdbm uses 1 for
 * success, 0 for error.
 */


WvQdbmHash::WvQdbmHash(WvStringParm dbfile, bool persist) : dbf(0)
{
    saveddata.dptr = 0;
    opendb(dbfile, persist);
}


WvQdbmHash::~WvQdbmHash()
{
    closedb();
}


void WvQdbmHash::dperr()
{
    seterr(dperrmsg(dpecode));
}


void WvQdbmHash::closedb()
{
    if (dbf)
    {
        char *dbfile = vlname((VILLA *) dbf);
        if (!vlclose((VILLA *) dbf)) dperr();
        if (!persist_dbfile)
            vlremove(dbfile);
        free(dbfile);
        dbf = 0;
    }
    free((void*)saveddata.dptr);
    saveddata.dptr = 0;
    seterr("The db is closed.");    // this won't overwrite an earlier err
}


void WvQdbmHash::opendb(WvStringParm dbfile, bool _persist)
{
    static char tmpbuf[] = "/var/tmp/qdbm-annoymousdb-XXXXXX";
    static int tmpbuf_inited = 0;
    closedb();

    noerr();

    persist_dbfile = _persist;
    //jdeboer: If this is an anonymous hash, we don't want it to persist!
    if (dbfile.isnull()) {
        persist_dbfile = false;
        int fd = mkstemp(tmpbuf);
        close(fd);
        tmpbuf_inited = 1;
    }
    if (tmpbuf_inited > 0 && dbfile == WvString(tmpbuf)) {
        // sometime people reopen the database with the same name.
        persist_dbfile = false;
    }

    int mode = VL_OWRITER | VL_OCREAT;
    if (!persist_dbfile) mode |= VL_OTRUNC;
    
    // VL_CMPLEX = lexigraphic comparison
    // FIXME: tmpnam!!
    
    const char *fname = dbfile.isnull() ? tmpbuf : dbfile.cstr();
    dbf = vlopen(fname, mode, VL_CMPLEX);
    if (dbf == NULL) dperr();
}


void WvQdbmHash::add(const datum &key, const datum &data, bool replace)
{
    if (!isok()) return;
    int r = vlput((VILLA*)dbf, key.dptr, key.dsize, data.dptr, data.dsize,
            replace ? VL_DOVER : VL_DKEEP);

    if (!r) dperr();
}


void WvQdbmHash::remove(const datum &key)
{
    if (!isok()) return;

    if (!vlout((VILLA*)dbf, key.dptr, key.dsize)) dperr();
}


WvQdbmHash::Datum WvQdbmHash::find(const datum &key)
{
    free((void*)saveddata.dptr);
    saveddata.dptr = NULL;

    if (isok())
        saveddata.dptr = vlget((VILLA*)dbf, key.dptr, key.dsize,
                &saveddata.dsize);

    return saveddata;
}


bool WvQdbmHash::exists(const datum &key)
{
    if (!isok()) return false;
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
    // FIXME: it does no error checking, cause half the time "errors" are just
    // "key not found".  Stupid API that doesn't distinguish these.
    if (!parent.isok())
        return;

    VILLA *dbf = (VILLA*)parent.dbf;
    Datum wanted = { 0, 0 };

    int ret;
    if (curkey.dptr)
    {
        // If we are given the current key, seek to it
        // but make a backup of the key first to check if the seek was
        // successful
        wanted.dsize = curkey.dsize;
        wanted.dptr = (typeof(wanted.dptr))malloc(wanted.dsize);
        memcpy((void*)wanted.dptr, curkey.dptr, wanted.dsize);
        ret = vlcurjump(dbf, curkey.dptr, curkey.dsize, VL_JFORWARD);
    }
    else if (rewindto.dptr)
        ret = vlcurjump(dbf, rewindto.dptr, rewindto.dsize, VL_JFORWARD);
    else
        ret = vlcurfirst(dbf);

    if (!ret) goto DONE;

    // obtain the current key
    curkey.dptr = vlcurkey((VILLA*)parent.dbf, &curkey.dsize);
    if (!curkey.dptr) goto DONE;

    // if current key is the same as the backup....
    if (wanted.dptr && wanted.dsize == curkey.dsize
            && !memcmp(wanted.dptr, curkey.dptr, wanted.dsize))
    {
        // the seek was successful. so go forward one more, and re-obtain the
        // key.
        if (vlcurnext(dbf))
            curkey.dptr = vlcurkey((VILLA*)parent.dbf, &curkey.dsize);
        else
            curkey.dptr = NULL;

        // FIXME: this assumes that any false result means "not found", not
        // error
    }

    // if we managed to get a valid key, fetch the data for it
    if (curkey.dptr)
        curdata = parent.find(curkey);

DONE:
    free((void*)wanted.dptr);

    //FIXME: yowza.  This needs an assert.
}

#endif
