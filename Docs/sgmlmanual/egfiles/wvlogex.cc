/** \file
 * A WvLog example.
 */
/** \example wvlogex.cc
 * Some text about this example...
 */
#include <wvstreamlist.h>
#include <wvpipe.h>
#include <wvlog.h>

void concallback(WvStream &con, void *userdata)
{
    WvStream &p = *(WvStream *)userdata;

    char *str = con.getline(0);
    if (str)
        p.print("%s\n", str); //
}

int main()
{
    const char *argv1[] = { "sh", "-c", "while :; do echo foo; sleep 3; done", NULL };

    WvLog log1("logger_1", WvLog::Info);
    WvPipe pipe1(argv1[0], argv1, false, true, false);

    pipe1.autoforward(log1);
    wvcon->setcallback(concallback, wvout);

    WvStreamList l;
    l.append(&pipe1, false);
    l.append(wvcon, false);

    while (wvcon->isok())
    {
        if (l.select(1000))
            l.callback();
        else
            log1("[TICK]\n");
    }
}