#include "wvfam.h"

#ifdef WITH_FAM

#include "wvistreamlist.h"
#include <sys/stat.h>

void WvFamBase::close()
{
    if (!s)
        return;

    WvIStreamList::globallist.unlink(s);
    delete s;
    s = 0;

    if (FAMClose(&fc) == -1)
        log(WvLog::Error, "%s\n", FamErrlist[FAMErrno]);
}

bool WvFamBase::isok() const
{
    if (s && s->isok())
        return true;

    return false;
}

int WvFamBase::_monitordir(WvString *dir)
{
    if (isok() && !FAMMonitorDirectory(&fc, *dir, &fr, dir))
        return fr.reqnum;

    log(WvLog::Error, "Could not monitor directory '%s'.\n", *dir);
    return -1;
}

int WvFamBase::_monitorfile(WvString *file)
{
    if (isok() && !FAMMonitorFile(&fc, *file, &fr, 0))
        return fr.reqnum;

    log(WvLog::Error, "Could not monitor file '%s'.\n", *file);
    return -1;
}

void WvFamBase::_unmonitor(int reqid)
{
    if (!isok())
        return;

    fr.reqnum = reqid;
    FAMCancelMonitor(&fc, &fr);
}

void WvFamBase::_callback()
{
    int famstatus;

    while((famstatus = FAMPending(&fc)) && famstatus != -1
        && FAMNextEvent(&fc, &fe) > 0)
    {
        if (fe.code == FAMChanged || fe.code == FAMDeleted
                || fe.code == FAMCreated)
        {
            if (!fe.userdata)
                cb(WvString(fe.filename), WvFamEvent(fe.code), false);

            // If the void * points to something this is a directory callback.
            // We'll prepend the path to the returned filename.
            else
                cb(WvString("%s/%s",
                    *reinterpret_cast<WvString *>(fe.userdata),
                    fe.filename), WvFamEvent(fe.code), true);
        }
    }

    if (famstatus == -1)
        log(WvLog::Error, "%s\n", FamErrlist[FAMErrno]);
}

bool WvFamBase::fam_ok()
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

void WvFamBase::setup()
{
    if (FAMOpen(&fc) == -1)
    {
        log(WvLog::Error, "Could not connect to FAM: %s\n",
                FamErrlist[FAMErrno]);
    }
    else
    {
        s = new WvFDStream(fc.fd);

        s->setcallback(WvStreamCallback(this, &WvFamBase::callback), 0);

        WvIStreamList::globallist.append(s, false);
    }
}

void WvFam::monitordir(WvStringParm dir)
{
    if (reqs[dir])
        return;

    WvFamReq *req = new WvFamReq(dir, 0, true);
    req->data = _monitordir(&req->key);

    if (req->data <= 0)
    {
        delete req;
        return;
    }

    reqs.add(req, true);
}

void WvFam::monitorfile(WvStringParm file)
{
    if (reqs[file])
        return;

    WvFamReq *req = new WvFamReq(file, 0, true);
    req->data = _monitorfile(&req->key);

    if (req->data <= 0)
    {
        delete req;
        return;
    }

    reqs.add(req, true);
}

void WvFam::monitor(WvStringParm path)
{
    struct stat buf;
    if (stat(path, &buf))
        return;

    if (S_ISDIR(buf.st_mode))
        monitordir(path);
    else
        monitorfile(path);
}

void WvFam::unmonitor(WvStringParm path)
{
    WvFamReq *req = reqs[path];
    if (!req)
        return;

    _unmonitor(req->data);
    reqs.remove(req);
}

#endif /* WITH_FAM */
