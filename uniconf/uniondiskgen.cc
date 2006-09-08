/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2006 Net Integration Technologies, Inc.
 * 
 * Implements a UniConf generator that stores its data on the filesystem.
 */ 

#include "wvautoconf.h"

#ifdef WITH_QDBM

#include "uniondiskgen.h"
#include "uniconf.h"
#include "unilistiter.h"
#include "wvmoniker.h"
#include "wvtclstring.h"

// FIXME
#include "wvlog.h"

extern "C" {
#include <stdlib.h>
#include <unistd.h>
}

void dump_db(VILLA *dbh)
{
    vlcurfirst(dbh);
    char *key = vlcurkey(dbh, NULL);
    char *val = vlcurval(dbh, NULL);
    wvcon->print("Database Contents:\n");
    wvcon->print("%s = %s\n", key, val);
    free(key), free(val);
    while (vlcurnext(dbh))
    {
        key = vlcurkey(dbh, NULL);
        val = vlcurval(dbh, NULL);
        wvcon->print("%s = %s\n", key, val);
        free(key), free(val);
    }
}

int qdbm_casecmp(const char *aptr, int asiz, const char *bptr, int bsiz)
{
    if (*aptr == '\0' && *bptr == '\0')
        return 0;
    else if (*aptr == '\0')
        return -1;
    else if (*bptr == '\0')
        return 1;
    else
        return strcasecmp(aptr, bptr);
}


static IUniConfGen *creator(WvStringParm args, IObject *, void *)
{
    WvConstInPlaceBuf buf(args, args.len());
    WvString file(wvtcl_getword(buf));
    if (!file) file = "";

    return new UniOnDiskGen(file);
}

static WvMoniker<IUniConfGen> reg("ondisk", creator);

/***** UniOnDiskGen *****/

void UniOnDiskGen::init(WvStringParm filename)
{
    persist = true; 
    dbfile = filename; 

    if (!filename)
    {
        dbfile = "/var/tmp/uni-ondisk-db-XXXXXX";
        int fd = mkstemp(dbfile.edit());
        assert(fd != -1);
        close(fd);
        // Need to clobber file in this case
        persist = false;
    }
    
    // FIXME: VL_OLCKNB seems to require a newer QDBM than 1.7.7
    dbh = vlopen(dbfile, VL_OWRITER | VL_OCREAT | VL_OTRUNC /*| VL_OLCKNB*/, 
            qdbm_casecmp);
    WvLog log("foo");
    log("Trying to open database file %s, got dbh=%s\n", 
            dbfile, (unsigned long)dbh);

    if (!dbh)
    {
        log("Could not open db! Error: %s\n", dperrmsg(dpecode));
        // FIXME: Report error better, implement isok()?
        //seterr(dperrmsg(dpecode));
    }
}


UniOnDiskGen::~UniOnDiskGen()
{
    vlclose(dbh);
    if (!persist && !!dbfile)
        unlink(dbfile);
}


bool UniOnDiskGen::exists_str(const WvString keystr)
{
#if 0
    WvLog log("Exists", WvLog::Error);
    log("Checking %s\n", keystr);
#endif

    int num_recs = vlvnum(dbh, keystr, -1);
    return num_recs != 0;
}


WvString UniOnDiskGen::get(const UniConfKey &key)
{
#if 0
    WvLog log("Get", WvLog::Error);
    log("Getting %s\n", key.printable());
#endif

    // Look for an empty section at the end.
    if (!key.isempty() && key.last().isempty())
        return WvString::null;

    WvString keystr(key.printable());

    WvString retval(WvString::null);
    char *val = vlget(dbh, keystr, -1, NULL);
    if (val != NULL)
    {
        retval = cache.get(val);
        free(val);
    }

    //dump_db(dbh);
    return retval;
}


void UniOnDiskGen::set(const UniConfKey &key, WvStringParm value)
{
    hold_delta();
    WvString keystr(key.printable());

#if 0
    WvLog log("Set", WvLog::Error);
    log("Setting %s to %s\n", keystr, WvString(value));
#endif

    if (!value.isnull())
    {
        UniConfKey mykey(key.removelast(1));
        // Autovivify any required elements
        if (mykey.numsegments() > 0 && !exists(mykey))
            set(mykey, WvString::empty);

        char *oldval = vlget(dbh, keystr, -1, NULL);
        if (oldval == NULL || WvFastString(oldval) != value)
        {
            vlput(dbh, keystr, -1, value, -1, VL_DOVER);
            delta(key, value);
        }

        if (oldval != NULL)
            free(oldval);
    }
    else
    {
        // Delete the subtree
        hold_delta();
        UniConfGen::Iter *it = recursiveiterator(key);
        for (it->rewind(); it->next(); )
        {
            // FIXME: UniConfKey.append is weird, may be a bug.
            UniConfKey delkey(it->key());
            delkey.prepend(key);
            // FIXME: Might not be needed if we could append it->key() onto
            // key instead of this prepend baloney
            if (it->key().numsegments() == 0)
                delkey.append("/");
#if 0
            log("Also deleting %s\n", delkey.printable());
#endif
            WvString delkeystr(delkey.printable());
            vlout(dbh, delkeystr, -1);
            delta(delkeystr, WvString::null);
        }
        delete it;
        unhold_delta();

        int num_recs = vlvnum(dbh, keystr, -1);
        if (num_recs > 0)
        {
            vlout(dbh, keystr, -1);
            delta(key, value);
        }
    }

    unhold_delta();

    //dump_db(dbh);
}


void UniOnDiskGen::setv(const UniConfPairList &pairs)
{
    setv_naive(pairs);
}


bool UniOnDiskGen::haschildren(const UniConfKey &key)
{
    bool retval = false;
    WvString keystr(key.printable());

#if 0
    WvLog log("haschildren", WvLog::Error);
    log("Checking children of %s\n", keystr);
#endif
    
    vlcurjump(dbh, keystr, -1, VL_JFORWARD);
    char *curkey = vlcurkey(dbh, NULL);
    //log("curkey=%s, keystr=%s\n", curkey, keystr);
    if (curkey != NULL && strcasestr(curkey, keystr.cstr()) == curkey)
    {
        // It's possible that vlcurjump() already took us to the next key,
        // if we're checking for haschildren("/")
        if (strcmp(curkey, keystr.cstr()) == 0)
            vlcurnext(dbh);
        char *nextkey = vlcurkey(dbh, NULL);

        if (nextkey != NULL && strcasestr(nextkey, keystr.cstr()) == nextkey)
            retval = true;

        if (nextkey != NULL)
            free(nextkey);
    }

    //log("Returning %s\n", retval);
    if (curkey != NULL)
        free(curkey);
    return retval;
}


UniConfGen::Iter *UniOnDiskGen::iterator(const UniConfKey &key)
{
    WvString keystr(key.printable());

#if 0
    WvLog log("iterator", WvLog::Error);
    log("Iterating over %s\n", keystr);
#endif

    ListIter *it = new ListIter(this);

    if (vlcurjump(dbh, keystr, -1, VL_JFORWARD))
        do
        {
            // Check if it's a (strict) subkey of the given key, and it itself has
            // no subkeys
            // FIXME: Deal with trailing slashes properly

            char *curkey = vlcurkey(dbh, NULL);
            char *subloc = strcasestr(curkey, keystr.cstr()); 
            char *cursubkey = curkey + keystr.len();

            // Check if we've passed any possible place for a subkey
            if (subloc == NULL)
            {
                free(curkey);
                break;
            }

            if (subloc == curkey && strlen(cursubkey) > 0)
            {
                char *lastslash = strrchr(cursubkey, '/');

                if (lastslash == NULL || lastslash == cursubkey)
                {
                    // FIXME: This allocates memory to store the key string
                    // twice - once by qdbm, and once by WvString.  This could
                    // be avoided by hacking the value returned by vlcurkey()
                    // (or vlcurval()) into a WvStringBuf ourselves, and
                    // linking that into a WvString.  But that's a bit scary,
                    // so don't do it unless it becomes a performance problem
                    // otherwise.
                    // 
                    // UPDATE: You can't assign to a WvStringBuf, you have to
                    // allocate one at the same time as the char*.  Mashing
                    // this into QDBM would be disgusting.
                    char* curval = vlcurval(dbh, NULL);
#if 0
                    log("Adding key=%s, val=%s\n", cursubkey, curval);
#endif
                    it->add(cache.get(cursubkey), cache.get(curval));
                    free(curval);
                }
            }

            free(curkey);
        } while (vlcurnext(dbh));

    return it;
}


UniConfGen::Iter *UniOnDiskGen::recursiveiterator(const UniConfKey &key)
{
    WvString keystr(key.printable());

#if 0
    WvLog log("Recursive Iterator", WvLog::Error);
    log("Recursively iterating over %s\n", keystr);
#endif

    ListIter *it = new ListIter(this);

    if (vlcurjump(dbh, keystr, -1, VL_JFORWARD))
        do
        {
            // Check if it's a (strict) subkey of the given key, and it itself
            // has no subkeys
            // FIXME: Deal with trailing slashes properly
            char *curkey = vlcurkey(dbh, NULL);
            char *subloc = strcasestr(curkey, keystr.cstr()); 

#if 0
            log("keystr:%s; subloc:%s; itkey:%s;\n",
                    keystr,
                    subloc == NULL ? "NULL" : subloc, 
                    itkey);
#endif
            
            // Check if we've passed any possible place where there'd be a
            // subkey
            if (subloc == NULL && !!keystr && curkey != NULL && *curkey != '\0')
            {
                free(curkey);
                break;
            }

            char *cursubkey = curkey + keystr.len();
            if (subloc == curkey && strlen(cursubkey) > 0)
            {
                char *curval = vlcurval(dbh, NULL);
#if 0
                log("Adding key=%s, val=%s\n", cursubkey, curval);
#endif
                it->add(cache.get(cursubkey), cache.get(curval));
                free(curval);
            }

            free(curkey);
        } while (vlcurnext(dbh));

    return it;
}

#endif
