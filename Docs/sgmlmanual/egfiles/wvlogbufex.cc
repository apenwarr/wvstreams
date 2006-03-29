/*
 * A WvLogBuf example.
 *
 * The expected output is :
 *
 * 1043174856 logA<*1>: a message+
 * 1043174856 logB<*2>: b message+
 * 1043174856 logC<*3>: c message with extra newline+
 * 1043174856 logC<*4>: c2 message+
 * 1043174856 logA<Info>: a info message+
 * 1043174856 logA<*1>: a normal message with [07][08] control chars+
 * 1043174856 logA<*1>: a split+
 * 1043174856 logB<*2>: message with stuff+
 * 1043174856 logB<Info>: and other stuff.+
 * 1043174856 logC<*3>: another split message.+
 *
 *
 * 1043174856 logA<*1>: a message+
 * 1043174856 logB<*2>: b message+
 * 1043174856 logC<*3>: c message with extra newline+
 * 1043174856 logC<*4>: c2 message+
 * 1043174856 logA<Info>: a info message+
 * 1043174856 logA<*1>: a normal message with [07][08] control chars+
 * 1043174856 logA<*1>: a split+
 * 1043174856 logB<*2>: message with stuff+
 * 1043174856 logB<Info>: and other stuff.+
 * 1043174856 logC<*3>: another split message.+
 * 1043174856 logC<*3>: .. and it's wonky!+
 *
 */

#include "wvlogbuffer.h"

int main()
{
    WvLogBuffer rc(4);

    WvLog a("logA", WvLog::Debug), b("logB", WvLog::Debug2);
    WvLog c("logC", WvLog::Debug3), c2 = c.split(WvLog::Debug4);

    a("a message\n");
    b("b message\n");
    c("c message with extra newline\n\n\n"); // extra newline discarded
    c2("c2 message\n");

    // the second line should be back at WvLog::Debug
    a(WvLog::Info, "a info message\n");
    a("a normal message with \a\b control chars\r\n");

    // should display like this:
    //  a split // message with stuff // and other stuff
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
