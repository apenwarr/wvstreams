#include "wvfam.h"

#ifdef WITH_FAM

#include "wvistreamlist.h"

WvFAM::WvFAM() : cb(NULL), log("WvFAM"), reqs(0)
{
    if (FAMOpen(fc) == -1)
        log(WvLog::Critical, "Could not connect to FAM!\n");
    else
    {
        s = new WvFDStream(fc->fd);

        s->setcallback(wvcallback(WvStreamCallback,
            *this, WvFAM::Callback), 0);

        WvIStreamList::globallist.append(s, true);
    }
}

WvFAM::~WvFAM()
{
    WvIStreamList::globallist.unlink(s);

    if (FAMClose(fc) == -1)
        log(WvLog::Critical, "Could not disconnect from FAM...? Huh...?\n");
}

bool WvFAM::isok()
{
    if (s && s->isok())
        return true;

    return false;
}

void WvFAM::setcallback(FAMCallback _cb, void *userdata)
{
    cb = _cb;
    cbdata = userdata;    
}

void WvFAM::monitordir(WvStringParm dir)
{
    if (!FAMMonitorDirectory(fc, dir, &fr, NULL))
        reqs.add(dir, fr.reqnum);
    else
        log(WvLog::Error, "Could not monitor directory '%s'.\n", dir);
}

void WvFAM::monitorfile(WvStringParm file)
{
    if (!FAMMonitorFile(fc, file, &fr, NULL))
        reqs.add(file, fr.reqnum);
    else
        log(WvLog::Error, "Could not monitor file '%s'.\n", file);
}

void WvFAM::unmonitordir(WvStringParm dir)
{
    if (!reqs.exists(dir))
        return;

    fr.reqnum = reqs[dir];    
    FAMCancelMonitor(fc, &fr);
}

void WvFAM::Callback(WvStream &, void *)
{

}

#endif
