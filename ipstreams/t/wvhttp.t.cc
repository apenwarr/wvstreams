#include "wvhttp.h"
#include "wvistreamlist.h"
#include "wvstrutils.h"
#include "wvtest.h"


static void tcp_callback(WvStream &s, void *userdata)
{
    char *line;
    do
    {
        line = s.getline();
        if (line && (strncmp(line, "GET", 3) == 0 ||
                     strncmp(line, "HEAD", 4) == 0))
        {
            WvStringList l;
            l.split(line);
            l.popstr(); // GET/HEAD
            WvString item = &(l.popstr().cstr()[1]); // chop off /
            wvcon->print("User allegedly wants (%s)\n", item);
            WVPASSEQ(item, "FOO");

            printf("Sending 200\n");
            s.write(WvString("HTTP/1.1 200 OK\n"
                             "Content-Length: 5\n"
                             "Content-Type: data\n\n"));
            s.write("Foo!\n");
        }
    } while (line);

    s.close();
}


static void listener_callback(WvStream &s, void *userdata)
{
    WvTCPListener &l = (WvTCPListener &)s;
    WvTCPConn *newconn = l.accept();
    newconn->setcallback(tcp_callback, userdata);
    WvIStreamList::globallist.append(newconn, true, "incoming http conn");
}


static bool got_file = false;

static void http_callback(WvStream &s, void *userdata)
{
    char * line = s.getline();
    if (line)
    {
        WVPASSEQ(trim_string(line), "Foo!");
    }
}


WVTEST_MAIN("wvhttpstream basic")
{
    int port = 4321 + (rand() % 10);
    WvTCPListener listener(port);
    listener.setcallback(listener_callback, NULL);
    WvIStreamList::globallist.append(&listener, false);

    WvUrl url(WvString("http://localhost:%s/FOO", port));
    WvHTTPStream http(url);
    http.setcallback(http_callback, NULL);
    WvIStreamList::globallist.append(&http, false);

    while (!got_file)
        WvIStreamList::globallist.runonce();
}
