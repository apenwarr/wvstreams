#include <wvstreamlist.h>
#include <wvpipe.h>
#include <wvlog.h>
#include <wvmodem.h>

void concallback(WvStream &con, void *userdata)
{
    WvStream &modem = *(WvStream *)userdata;
    
    char *str = con.getline(0);
    if (str)
	modem.print("%s\r", str); // modems like CR, not newline
}

int main()
{
    const char *argv1[] = { "sh", "-c",
			    "while :; do echo foo; sleep 3; done", NULL };
    const char *argv2[] = { "sh", "-c",
			    "while :; do echo snorkle; sleep 2; done", NULL };

    WvLog log("logger", WvLog::Info);
    WvLog modemlog("modem", WvLog::Info);
    WvPipe pipe1(argv1[0], argv1, false, true, false);
    WvPipe pipe2(argv2[0], argv2, false, true, false);
    WvModem modem("/dev/ttyS2", O_RDWR);

    pipe1.autoforward(log);
    pipe2.autoforward(log);
    wvcon->setcallback(concallback, &modem);
    modem.autoforward(modemlog);
    
    WvStreamList l;
    l.append(&pipe1, false);
    l.append(&pipe2, false);
    l.append(&modem, false);
    l.append(wvcon, false);
    
    if (!modem.isok())
	modemlog(WvLog::Error, "%s\n", modem.errstr());
    
    while (wvcon->isok())
    {
	if (l.select(1000))
	    l.callback();
	else
	    log("[TICK]\n");
    }
}
