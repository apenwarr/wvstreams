/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * A generator for .ini files.
 */
#ifndef __UNICONFINITREEGEN_H
#define __UNICONFINITREEGEN_H

#include "uniconfgen.h"
#include "unitempgen.h"
#include "uniinigen.h"
#include "unifiletreegen.h"
#include "wvlog.h"

/**
 * Loads and saves ".ini"-style files similar to those used by
 * Windows which are stored in a directory.
 * 
 * To mount, use the moniker prefix "initree:" followed by the
 * directory to search.
 * 
 */
class UniIniTreeGen : public UniFileTreeGen
{
    static const WvString moniker;
public:
    /**
     * Creates a generator which can load/modify/save a .ini file.
     * "filename" is the local path of the .ini file
     */
    UniIniTreeGen(WvStringParm directory, bool recursive);

    virtual ~UniIniTreeGen();
    
    /***** Overridden members *****/

/*    virtual bool commit(const UniConfKey &key, UniConfDepth::Type depth);
    virtual bool refresh(const UniConfKey &key, UniConfDepth::Type depth);*/

private:
/*    void save(WvStream &file, UniConfValueTree &parent);
    bool refreshcomparator(const UniConfValueTree *a,
        const UniConfValueTree *b, void *userdata); */
};


#endif // __UNICONFINI_H
