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


/*
 * The WvFAMBase class is provided for efficiency. If you're going to be keeping
 * track of the stuff you're monitoring anyways then there's no reason to have
 * duplicate wvstrings/hashes. This class accepts a pointer to a wvstring and
 * returns the request id number (which is needed to unmonitor).
 *
 * If you're not keeping a list of stuff you're monitoring around for other
 * reasons just ignore this and use the main WvFAM class.
 */
class WvFAMBase
{
protected:

    // These calls all take a pointer to a WvString. The WvString must exist and
    // be unmodified until the fam monitoring is removed for directory
    // monitoring. For file monitoring you can get rid of it, but you'll
    // probably want to use it for tracking ids anyways.
    int _monitordir(WvString *dir);
    int _monitorfile(WvString *file);

    void _unmonitor(int reqid);

    FAMConnection fc;
    FAMRequest fr;
    FAMEvent fe;
    FAMCallback cb;

    WvFDStream *s;
    WvLog log;

    void callback(WvStream &, void *) { callback(); }
    void callback();

    void setup();

public:
    WvFAMBase() : cb(NULL), s(0), log("WvFAM") { setup(); }
    WvFAMBase(FAMCallback _cb) : cb(_cb), s(0), log("WvFAM") { setup(); }
    ~WvFAMBase();

    static bool fam_ok();

    bool isok() const;

    void setcallback(FAMCallback _cb)
        { cb = _cb; }
};


class WvFAM : public WvFAMBase
{
public:
    WvFAM() { }
    WvFAM(FAMCallback _cb) : WvFAMBase(_cb) { }

    void monitordir(WvStringParm dir);
    void monitorfile(WvStringParm file);
    void monitor(WvStringParm path);
    void unmonitor(WvStringParm path);

protected:
    typedef WvMapPair<WvString, int> WvFAMReq;
    DeclareWvScatterDict2(WvFAMReqDict, WvFAMReq, WvString, key);
    WvFAMReqDict reqs;
};


#endif //__WVFAM_H
#endif //__WITH_FAM
