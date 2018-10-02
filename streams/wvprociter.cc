/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Process iterator.  Iterates through all the processes.
 *
 */

#include "wvprociter.h"
#include "wvpipe.h"
#include "wvstrutils.h"
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>


WvProcIter::WvProcIter()
{
    p = NULL;
}

WvProcIter::~WvProcIter()
{
    delete p;
}

bool WvProcIter::isok() const
{
    return !p || p->isok();
}

void WvProcIter::rewind()
{
    if (p) delete p;
    p = NULL;

    const char *argv[] = {"ps", "ax", "-o", "pid,command", NULL};
    p = new WvPipe(argv[0], argv, false, true, false);
    p->blocking_getline(-1);
    if (!p->isok())
    {
        int rc = p->finish();
	fprintf(stderr, "WARNING: empty results from ps: exit code %d\n", rc);
    }
}

bool WvProcIter::next()
{
    assert(p);
    if (!p || !p->isok()) return false;

    // Okay to use blocking_getline here because we don't expect ps to
    // take long to start returning data.
    char *line = p->blocking_getline(-1);
    if (!line) return false;

    WvString s(trim_string(line));
    WvStringList l;
    l.split(s, " ", 2);
    assert(l.count() == 2);

    WvStringList::Iter i(l);
    i.rewind(); i.next();
    proc_ent.pid = atol(*i);
    assert(proc_ent.pid > 0);
    i.next();
    proc_ent.cmdline.zap();
    proc_ent.cmdline.split(*i);
    proc_ent.exe = *proc_ent.cmdline.first();
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
