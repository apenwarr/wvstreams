#include "wvstreamsdaemon.h"
#include "wvstreamclone.h"
#include "wvtcp.h"
#include "wvlog.h"

const char * NAME = "MyWvStreamsDaemon";
const char * VERSION = "1.0";
const int default_port = 2121;


class MyClient : public WvStreamClone
{
public:
    MyClient(IWvStream *conn) :
        WvStreamClone(conn),
        log("MyClient", WvLog::Info) 
    {
        uses_continue_select = true;

        print("Hello I am %s v%s\n", NAME, VERSION);

        log(WvLog::Notice, "Connection from: %s\n", *conn->src());
    }
    virtual ~MyClient() { 
        log(WvLog::Notice, "Connection to %s closed!\n", *cloned->src());
        terminate_continue_select();
    }
    virtual void execute() {
        // FIXME: Handle carraige returns..
        WvStreamClone::execute();
        WvString line = blocking_getline(-1);
        if (!!line)
        {
            if (strncmp(line.cstr(), "quit", 4) == 0)
            {
                print("Goodbye!\n");
                flush_then_close(1000);
            }
            print("You said: %s\n", line);
        }
    }

private:
    WvLog log;
};


class MyListener : public WvTCPListener
{
public:
    MyListener(const WvIPPortAddr &addr) :
        WvTCPListener(addr),
        log("MyListener", WvLog::Info)
    {
        if (isok())
        {
            onaccept(wv::bind(&MyListener::accept_conn, this, _1));
            log("Listening for client connections on %s\n", addr);
        }
        else 
            log(WvLog::Error, 
                "Failed to listen for client connections on %s\n", addr);
    }

private:
    void accept_conn(IWvStream *s);
    WvLog log;
};


class MyWvStreamsDaemon : public WvStreamsDaemon
{
public:
    MyWvStreamsDaemon() :
        WvStreamsDaemon("MyWvStreamsDaemon", "1.0", 
                        wv::bind(&MyWvStreamsDaemon::cb, this)),
        port(default_port),
        log("MyWvStreamsDaemon", WvLog::Info)
    {
	args.add_option('p', "port", "specify alternate port number", 
			"PORT", port);
    }
    
    virtual ~MyWvStreamsDaemon() {}
    
    void cb()
    { 
        log("MyWvStreamsDaemon starting..\n", port);
        WvString bindto("0.0.0.0:%s", port);

        MyListener *l = new MyListener(bindto);
        add_stream(l, true, "echo port");
    }

private:
    int port;
    WvLog log;
};


static MyWvStreamsDaemon d;


void MyListener::accept_conn(IWvStream *s)
{
    log("Incoming TCP connection from %s.\n", *s->src());
    d.add_stream(new MyClient(s), true, "MyClient");
}


int main(int argc, char *argv[])
{
    return d.run(argc, argv);
}
