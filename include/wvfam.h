/* -*- Mode: C++ -*- */
#include "wvautoconf.h"

#ifdef HAVE_FAM_H

#ifndef __WVFAM_H
#define __WVFAM_H

#include "wvfdstream.h"
#include "wvlog.h"
#include "wvscatterhash.h"

#include <fam.h>

enum WvFamEvent
{
    WvFamChanged = 1,
    WvFamDeleted = 2,
    WvFamCreated = 5,
};

typedef WvCallback<void, WvStringParm, WvFamEvent, bool> WvFamCallback;


/*
 * The WvFamBase class is provided for efficiency. If you're going to
 * be keeping track of the stuff you're monitoring anyways then
 * there's no reason to have duplicate wvstrings/hashes. This class
 * accepts a pointer to a wvstring and returns the request id number
 * (which is needed to unmonitor).
 *
 * If you're not keeping a list of stuff you're monitoring around for
 * other reasons just ignore this and use the main WvFam class.
 */
class WvFamBase
{
public:
    // These calls all take a pointer to a WvString. The WvString must exist and
    // be unmodified until the fam monitoring is removed for directory
    // monitoring.
    //
    // This is really evil, but if you're going to be monitoring a lot of files
    // then duplicating the strings can be quite wasteful. 
    int _monitordir(WvString *dir);
    int _monitorfile(WvString *file);
    void _unmonitor(int reqid);

protected:
    FAMConnection fc;
    FAMRequest fr;
    FAMEvent fe;
    WvFamCallback cb;

    WvFDStream *s;
    WvLog log;

    void callback(WvStream &, void *) { _callback(); }
    void _callback();


public:
    WvFamBase() : s(0), log("WvFAM") { setup(); }
    WvFamBase(WvFamCallback _cb) : cb(_cb), s(0), log("WvFam") { setup(); }
    ~WvFamBase() { close(); }

    void setup();

/**
 * These should be the only calls from WvFamBase that most people ever need to
 * look at.
 */
    void close();

    static bool fam_ok();

    bool isok() const;

    void setcallback(WvFamCallback _cb)
        { cb = _cb; }
};



/**
 * The actual WvFam class that you should be using unless you really know what
 * you're doing and really have some reason for using WvFamBase.
 */
class WvFam : public WvFamBase
{
public:
    WvFam() { }
    WvFam(WvFamCallback _cb) : WvFamBase(_cb) { }

    void monitordir(WvStringParm dir);
    void monitorfile(WvStringParm file);
    void monitor(WvStringParm path);
    void unmonitor(WvStringParm path);

protected:
    typedef WvMapPair<WvString, int> WvFamReq;
    DeclareWvScatterDict2(WvFamReqDict, WvFamReq, WvString, key);
    WvFamReqDict reqs;
};


#endif //__WVFAM_H
#endif //__HAVE_FAM_H
