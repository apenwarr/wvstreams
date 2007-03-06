/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2006 Net Integration Technologies, Inc.
 * 
 * A UniConf generator that stores its data on the filesystem.
 */ 

#ifndef UNI_ON_DISK_GEN_H
#define UNI_ON_DISK_GEN_H

#include "wvautoconf.h"

#ifdef WITH_QDBM

#include "uniconfgen.h"
#include "wvstringcache.h"

extern "C" {
#include <villa.h>
}

// FIXME
#include "wvfile.h"

class UniOnDiskGen : public UniConfGen
{
public:
    UniOnDiskGen() { init(WvString::null); }
    UniOnDiskGen(WvStringParm filename) { init(filename); }
    virtual ~UniOnDiskGen();

    void init(WvStringParm filename);

    /***** Overridden members *****/

    virtual bool exists(const UniConfKey &key) 
    { 
        return exists_str(key.printable()); 
    }
    virtual bool haschildren(const UniConfKey &key);
    virtual WvString get(const UniConfKey &key);
    virtual void set(const UniConfKey &key, WvStringParm value);
    virtual void setv(const UniConfPairList &pairs);
    virtual Iter *iterator(const UniConfKey &key);
    virtual Iter *recursiveiterator(const UniConfKey &key);
    virtual void commit() 
    { 
        static int count = 0; 
        ++count;
        if ((count % 100) == 0)
            vlsync(dbh); 
        if ((count % 10000) == 0) 
            vloptimize(dbh); 
    }

protected:
    virtual void flush_buffers() { }
    virtual bool exists_str(WvString keystr);

private:
    VILLA *dbh;
    bool persist;
    WvString dbfile;
    WvStringCache cache;

    // FIXME
    WvFile logfile;
};

#endif
#endif
