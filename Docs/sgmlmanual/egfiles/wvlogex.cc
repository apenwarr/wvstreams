/** \file
 * A WvLogRcv example.
 */
/** \example wvlogrcvex.cc
 * Expected output:
 *	logA<*1>: a message
 *	logB<*2>: b message
 *	logB<*2>: b message
 *	logC<*3>: c message with extra newline
 *	logC<*4>: c2 message
 *	logA<Info>: a info message
 *	logA<*1>: a normal message with [07][08] control chars
 *	logA<*1>: a split
 *	logB<*2>: message with stuff
 *	logB<Info>: and other stuff.
 *	logC<*3>: another split message.
 */

#include "wvlogrcv.h"

int main()
{
    WvLog a("logA", WvLog::Debug), b("logB", WvLog::Debug2);
    WvLog c("logC", WvLog::Debug3), c2 = c.split(WvLog::Debug4);

    a("a message\n");
    b("b message\n"); // prints twice -- once for rc, once for rc2
    c("c message with extra newline\n\n"); // extra newline discarded
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

    // should auto-terminate line on exit

    return 0;
}
