#include "wvlogbuffer.h"
#include <unistd.h>

int main()
{
    WvLogBuffer rc(4);
    free(malloc(1));

    WvLog a("logA", WvLog::Debug), b("logB", WvLog::Debug2);
    WvLog c("logC", WvLog::Debug3), c2 = c.split(WvLog::Debug4);
    
    // default logging should disable itself while rc or rc2 exist
    // note: it is usually a bad idea to have more than one WvLogRx for
    //   the same output device!
    a("a message\n");
    b("b message\n"); // prints twice -- once for rc, once for rc2
    c("c message with extra newline\n\n"); // extra newline discarded
    c2("c2 message\n");
    
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
    
    // should auto-terminate line on display

    rc.dump(*wvcon);
    wvcon->print("\n\n");
    
    c2(WvLog::Debug3, "  .. and it's wonky!  \n");
    rc.dump(*wvcon);
    
    return 0;
}
