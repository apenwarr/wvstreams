#include "wvautoconf.h"

#ifdef WITH_FAM

#ifndef __WVFAM_H
#define __WVFAM_H

#include "wvfdstream.h"
#include "wvlog.h"
#include "wvscatterhash.h"

#include <fam.h>

enum WvFAMEvent
{
    Changed = 1,
    Deleted = 2,
    Created = 5,
};

DeclareWvCallback(3, void, FAMCallback, WvStringParm,
    enum WvFAMEvent, void *);


class WvFAM
{
public:
    WvFAM();
    ~WvFAM();
    bool isok();

    void setcallback(FAMCallback _cb, void *userdata);

    void monitordir(WvStringParm dir);
    void monitorfile(WvStringParm file);

    void unmonitordir(WvStringParm dir);
    void unmonitorfile(WvStringParm file)
        { unmonitordir(file); }


protected:
    FAMConnection *fc;
    FAMRequest fr;

    FAMCallback cb;
    void *cbdata;

    WvFDStream *s;
    WvLog log;

    WvMap<WvString, int, OpEqComp, WvScatterHash> reqs;

    void Callback(WvStream &, void *);
};


#endif //__WVFAM_H
#endif //__WITH_FAM
