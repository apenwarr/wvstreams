/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * A generator for .ini files.
 */
#ifndef __UNICONFINI_H
#define __UNICONFINI_H

#include "unitempgen.h"
#include "wvlog.h"
#include <sys/stat.h>

/**
 * Loads and saves ".ini"-style files similar to those used by
 * Windows, but adapted to represent keys and values using Tcl
 * style lists.
 * 
 * To mount, use the moniker prefix "ini:" followed by the
 * path of the .ini file.
 * 
 */
class UniIniGen : public UniTempGen
{
    WvString filename;
    int create_mode;
    WvLog log;
    struct stat old_st;
    
public:
    /**
     * Creates a generator which can load/modify/save a .ini file.
     * "filename" is the local path of the .ini file
     */
    UniIniGen(WvStringParm filename, int _create_mode = 0666);

    virtual ~UniIniGen();
    
    /***** Overridden members *****/

    virtual void commit();
    virtual bool refresh();

private:
    void save(WvStream &file, UniConfValueTree &parent);
    bool refreshcomparator(const UniConfValueTree *a,
        const UniConfValueTree *b, void *userdata);
};


#endif // __UNICONFINI_H
