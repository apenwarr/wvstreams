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
    WvFAMChanged = 1,
    WvFAMDeleted = 2,
    WvFAMCreated = 5,
};

DeclareWvCallback(2, void, FAMCallback, WvStringParm, WvFAMEvent);


class WvFAM
{
public:
    WvFAM() : cb(NULL), s(0), log("WvFAM"), reqs(0) { setup(); }
    WvFAM(FAMCallback _cb) : cb(_cb), s(0), log("WvFAM"), reqs(0) { setup(); }
    ~WvFAM();

    bool isok();

    void setcallback(FAMCallback _cb)
        { cb = _cb; }

    void monitordir(WvStringParm dir);
    void monitorfile(WvStringParm file);

    void unmonitordir(WvStringParm dir);
    void unmonitorfile(WvStringParm file)
        { unmonitordir(file); }


protected:
    FAMConnection fc;
    FAMRequest fr;
    FAMEvent fe;
    FAMCallback cb;

    WvFDStream *s;
    WvLog log;

    WvMap<WvString, int, OpEqComp, WvScatterHash> reqs;

    void Callback(WvStream &, void *);
    void setup();
};


#endif //__WVFAM_H
#endif //__WITH_FAM
