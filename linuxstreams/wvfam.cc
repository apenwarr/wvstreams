#include "wvfam.h"

#ifdef WITH_FAM

#include "wvistreamlist.h"
#include <sys/stat.h>

WvFAM::~WvFAM()
{
    WvIStreamList::globallist.unlink(s);

    if (FAMClose(&fc) == -1)
        log(WvLog::Error, "%s\n", FamErrlist[FAMErrno]);
}

bool WvFAM::isok() const
{
    if (s && s->isok())
        return true;

    return false;
}

void WvFAM::monitordir(WvStringParm dir)
{
    if (!isok() || reqs[dir])
        return;

    WvFAMReq *req = new WvFAMReq(dir, 0, true);

    if (!FAMMonitorDirectory(&fc, dir, &fr, &req->key))
    {
        req->data = fr.reqnum;
        reqs.add(req, true);
    }
    else
    {
        delete req;
        log(WvLog::Error, "Could not monitor directory '%s'.\n", dir);
    }
}

void WvFAM::monitorfile(WvStringParm file)
{
    if (!isok() || reqs[file])
        return;

    WvFAMReq *req = new WvFAMReq(file, 0, true);

    if (!FAMMonitorFile(&fc, file, &fr, NULL))
    {
        req->data = fr.reqnum;
        reqs.add(req, true);
    }
    else
    {
        delete req;
        log(WvLog::Error, "Could not monitor file '%s'.\n", file);
    }
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
    if (!isok())
        return;

    WvFAMReq *req = reqs[path];
    if (!req)
        return;

    fr.reqnum = req->data;
    FAMCancelMonitor(&fc, &fr);
    reqs.remove(req);
}

void WvFAM::callback()
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

bool WvFAM::fam_ok()
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

void WvFAM::setup()
{
    if (FAMOpen(&fc) == -1)
    {
        log(WvLog::Error, "Could not connect to FAM: %s\n",
                FamErrlist[FAMErrno]);
    }
    else
    {
        s = new WvFDStream(fc.fd);

        s->setcallback(wvcallback(WvStreamCallback, *this, WvFAM::callback), 0);

        WvIStreamList::globallist.append(s, true);
    }
}

#endif
