/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * WvHConfIniFile is a WvHConfGen for ".ini"-style files like the ones used
 * by Windows and the original WvConf.
 */
#ifndef __WVHCONFINI_H
#define __WVHCONFINI_H

#include "wvhconf.h"
#include "wvlog.h"


class WvHConfIniFile : public WvHConfGen
{
public:
    WvString filename;
    WvHConf *top;
    WvLog log;
    
    WvHConfIniFile(WvHConf *_top, WvStringParm _filename);
    virtual void load();
    virtual void save();
    
    void save_subtree(WvStream &out, WvHConf *h, WvHConfKey key);
};


#endif // __WVHCONFINI_H
