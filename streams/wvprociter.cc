/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Process iterator.  Iterates through all the processes.
 *
 */

#include "wvprociter.h"
#include "wvfile.h"
#include "wvfileutils.h"
#include <sys/types.h>
#include <signal.h>

WvProcIter::WvProcIter() :
    dir_iter("/proc", false, true)
{
    if (!dir_iter.isok())
	fprintf(stderr, "WARNING: Can't open /proc: is it mounted?\n");
    if (access("/proc/1/.", F_OK) != 0)
	fprintf(stderr, "WARNING: Can't find /proc/1: is /proc mounted?\n");
}

WvProcIter::~WvProcIter()
{
}

bool WvProcIter::isok() const
{
    return dir_iter.isok();
}

void WvProcIter::rewind()
{
    dir_iter.rewind();
}

bool WvProcIter::next()
{
    for (;;)
    {
        if (!dir_iter.next())
            return false;
        if (!wvstring_to_num(dir_iter->name, proc_ent.pid))
            continue;

        proc_ent.exe = wvreadlink(WvString("%s/exe", dir_iter->fullname));

        proc_ent.cmdline.zap();
        WvFile cmdline_file(WvString("%s/cmdline", dir_iter->fullname), O_RDONLY);
        while (cmdline_file.isok())
        {
            const char *line = cmdline_file.getline(0, '\0');
            if (line == NULL)
                break;
            WvString line_str(line);
            line_str.unique();
            proc_ent.cmdline.append(line_str);
        }
        cmdline_file.close();

        break;
    }
    return true;
}

bool wvkillall(WvStringParm name, int sig)
{
    bool found = false;
    WvProcIter i;
    for (i.rewind(); i.next(); )
    {
        if (!i->cmdline.isempty()
                && !!*i->cmdline.first()
                && getfilename(*i->cmdline.first()) == name
                && i->pid > 0)
        {
            ::kill(i->pid, sig);
            found = true;
        }
    }
    return found;
}
