#include "wvfam.h"

#ifdef WITH_FAM

#include "wvistreamlist.h"

WvFAM::~WvFAM()
{
    WvIStreamList::globallist.unlink(s);

    if (FAMClose(&fc) == -1)
        log(WvLog::Critical, "Could not disconnect from FAM...? Huh...?\n");
}

bool WvFAM::isok()
{
    if (s && s->isok())
        return true;

    return false;
}

void WvFAM::monitordir(WvStringParm dir)
{
    if (!isok())
        return;

    if (!FAMMonitorDirectory(&fc, dir, &fr, NULL))
        reqs.add(dir, fr.reqnum);
    else
        log(WvLog::Error, "Could not monitor directory '%s'.\n", dir);
}

void WvFAM::monitorfile(WvStringParm file)
{
    if (!isok())
        return;

    if (!FAMMonitorFile(&fc, file, &fr, NULL))
        reqs.add(file, fr.reqnum);
    else
        log(WvLog::Error, "Could not monitor file '%s'.\n", file);
}

void WvFAM::unmonitordir(WvStringParm dir)
{
    if (!isok())
        return;

    if (!reqs.exists(dir))
        return;

    fr.reqnum = reqs[dir];    
    FAMCancelMonitor(&fc, &fr);
    reqs.remove(dir);
}

void WvFAM::Callback(WvStream &, void *)
{
    int famstatus;

    while((famstatus = FAMPending(&fc)) && famstatus != -1)
    {
        if (FAMNextEvent(&fc, &fe) == 1)
        {
            if (fe.code == FAMChanged || fe.code == FAMDeleted
                    || fe.code == FAMCreated)
                cb(WvString(fe.filename), WvFAMEvent(fe.code));
        }
        else
            log("Odd... recieved event pending but no event.\n");
    }

    if (famstatus == -1)
        log(WvLog::Error, "FAM error: (%s)\n", FAMErrno);
}

void WvFAM::setup()
{
    if (FAMOpen(&fc) == -1)
    {
        log(WvLog::Critical, "Could not connect to FAM!\n", fc.fd);
        log(WvLog::Error, "FAM error: (%s)\n", FAMErrno);
    }
    else
    {
        s = new WvFDStream(fc.fd);

        s->setcallback(wvcallback(WvStreamCallback,
            *this, WvFAM::Callback), 0);

        WvIStreamList::globallist.append(s, true);
    }
}

#endif
