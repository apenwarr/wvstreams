#include "wvfam.h"

#ifdef WITH_FAM

#include "wvistreamlist.h"
#include <sys/stat.h>

WvFAMBase::~WvFAMBase()
{
    WvIStreamList::globallist.unlink(s);

    if (FAMClose(&fc) == -1)
        log(WvLog::Error, "%s\n", FamErrlist[FAMErrno]);
}

bool WvFAMBase::isok() const
{
    if (s && s->isok())
        return true;

    return false;
}

int WvFAMBase::_monitordir(WvString *dir)
{
    if (isok() && !FAMMonitorDirectory(&fc, *dir, &fr, dir))
        return fr.reqnum;

    log(WvLog::Error, "Could not monitor directory '%s'.\n", *dir);
    return -1;
}

int WvFAMBase::_monitorfile(WvString *file)
{
    if (isok() && !FAMMonitorFile(&fc, *file, &fr, 0))
        return fr.reqnum;

    log(WvLog::Error, "Could not monitor file '%s'.\n", *file);
    return -1;
}

void WvFAMBase::_unmonitor(int reqid)
{
    if (!isok())
        return;

    fr.reqnum = reqid;
    FAMCancelMonitor(&fc, &fr);
}

void WvFAMBase::callback()
{
    int famstatus;

    while((famstatus = FAMPending(&fc)) && famstatus != -1
        && FAMNextEvent(&fc, &fe) > 0)
    {
        if (fe.code == FAMChanged || fe.code == FAMDeleted
                || fe.code == FAMCreated)
        {
            if (!fe.userdata)
                cb(WvString(fe.filename), WvFAMEvent(fe.code));

            // If the void * points to something this is a directory callback.
            // We'll prepend the path to the returned filename.
            else
                cb(WvString("%s/%s",
                    *reinterpret_cast<WvString *>(fe.userdata),
                    fe.filename), WvFAMEvent(fe.code));
        }
    }

    if (famstatus == -1)
        log(WvLog::Error, "%s\n", FamErrlist[FAMErrno]);
}

bool WvFAMBase::fam_ok()
{
    FAMConnection fc;

    if (FAMOpen(&fc) == -1)
    {
        fprintf(stderr, "Error connecting to FAM: %s\n", FamErrlist[FAMErrno]);
        return false;
    }
    if (FAMClose(&fc) == -1)
    {
        fprintf(stderr, "Error diconnecting from FAM: %s\n",
                FamErrlist[FAMErrno]);
        return false;
    }
    return true;
}

void WvFAMBase::setup()
{
    if (FAMOpen(&fc) == -1)
    {
        log(WvLog::Error, "Could not connect to FAM: %s\n",
                FamErrlist[FAMErrno]);
    }
    else
    {
        s = new WvFDStream(fc.fd);

        s->setcallback(wvcallback(WvStreamCallback, *this, WvFAMBase::callback), 0);

        WvIStreamList::globallist.append(s, true);
    }
}

void WvFAM::monitordir(WvStringParm dir)
{
    if (reqs[dir])
        return;

    WvFAMReq *req = new WvFAMReq(dir, 0, true);
    req->data = _monitordir(&req->key);

    if (req->data <= 0)
    {
        delete req;
        return;
    }

    reqs.add(req, true);
}

void WvFAM::monitorfile(WvStringParm file)
{
    if (reqs[file])
        return;

    WvFAMReq *req = new WvFAMReq(file, 0, true);
    req->data = _monitorfile(&req->key);

    if (req->data <= 0)
    {
        delete req;
        return;
    }

    reqs.add(req, true);
}

void WvFAM::monitor(WvStringParm path)
{
    struct stat buf;
    if (stat(path, &buf))
        return;

    if (S_ISDIR(buf.st_mode))
        monitordir(path);
    else
        monitorfile(path);
}

void WvFAM::unmonitor(WvStringParm path)
{

}

#endif
