/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * UniConfIniFile is a UniConfGen for ".ini"-style files like the ones used
 * by Windows and the original WvConf.
 */
#ifndef __UNICONFINI_H
#define __UNICONFINI_H

#include "uniconf.h"
#include "wvlog.h"


class UniConfIniFile : public UniConfGen
{
public:
    WvString filename;
    UniConf *top;
    WvLog log;
    bool save_test; // test mode: don't actually save over input file
    
    // Pass false to the automount parameter if you don't want to automatically set
    // _top's generator to this.
    UniConfIniFile(UniConf *_top, WvStringParm _filename, bool automount);
    virtual void load();
    virtual void save();
    
    void save_subtree(WvStream &out, UniConf *h, UniConfKey key);
};


#endif // __UNICONFINI_H
