/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */

/** \file
 * A generator for .ini files.
 */
#ifndef __UNICONFINI_H
#define __UNICONFINI_H

#include "uniconf.h"
#include "wvlog.h"

/**
 * Loads and saves ".ini"-style files similar to those used by
 * Windows, but adapted to represent keys and values using Tcl
 * style lists.
 * <p>
 * To mount, use the moniker prefix "ini://" followed by the
 * path of the .ini file.
 * </p>
 */
class UniConfIniFileGen : public UniConfGen
{
public:
    WvString filename;
    UniConf *top;
    WvLog log;
    
    UniConfIniFileGen(UniConf *_top, WvStringParm _filename);
    virtual ~UniConfIniFileGen();
    virtual void load();
    virtual void save();
    
    void save_subtree(WvStream &out, UniConf *h, UniConfKey key);
};


/**
 * A factory for UniConfIniFileGen instances.
 */
class UniConfIniFileGenFactory : public UniConfGenFactory
{
public:
    virtual UniConfGen *newgen(const UniConfLocation &location,
        UniConf *top);
};


#endif // __UNICONFINI_H
