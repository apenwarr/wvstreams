/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997, 1998, 1999 Worldvisions Computer Technology, Inc.
 *
 * WvSyslog test.  Sends information to syslog.
 */

#include "wvsyslog.h"
#include <unistd.h>

int main()
{
    WvLogRcv *rc, *rc2;
    free(malloc(1));
    
    WvLog a("logA", WvLog::Debug), b("logB", WvLog::Debug2);
    WvLog c("logC", WvLog::Debug3), c2 = c.split(WvLog::Debug4);

    // default logging should disable itself while rc or rc2 exist
    // note: it is usually a bad idea to have more than one WvLogRx for
    //   the same output device!
    rc = new WvSyslog("testy", false);
    a("a message\n");
    rc2 = new WvSyslog("testy2", true);
    b("b message\n"); // prints twice -- once for rc, once for rc2
    delete rc;
    c("c message with extra newline\n\n"); // extra newline discarded
    c2("c2 message\n");
    delete rc2;
    
    rc = new WvSyslog("goople", false);
    
    // the second line should be back at WvLog::Debug
    a(WvLog::Info, "a info message\n");
    a("a normal message with \a\b control chars\r\n");
    
    // should display like this:
    //    a split // message with stuff // and other stuff
    a("a split ");
    b("message ");
    b("with stuff ");
    b(WvLog::Info, "and other stuff.\n");
    
    // should display all on one line
    c("another split ");
    c2(WvLog::Debug3, "message.");
    
    // should auto-terminate line on exit
    
    delete rc;
    return 0;
}
