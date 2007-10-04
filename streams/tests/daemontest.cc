#include "wvdaemon.h"
#include "wvlog.h"
#ifdef _WIN32
#include "streams.h"
#endif

class MyWvDaemon : public WvDaemon
{
public:
    MyWvDaemon() :
        WvDaemon("MyWvDaemon", "1.0", 
                 wv::bind(&MyWvDaemon::start_cb, this),
                 wv::bind(&MyWvDaemon::run_cb, this),
                 wv::bind(&MyWvDaemon::stop_cb, this)),
        tick_interval(1),
        log("MyWvDaemon", WvLog::Info)
        {
            args.add_option('i', "interval", "specify tick interval", 
                            "SECONDS", tick_interval);
        }
    virtual ~MyWvDaemon() {}
    void start_cb() { log("start callback\n"); }
    void run_cb() {
        log("run callback\n"); 
        while (should_run()) {
            sleep(tick_interval);
            log("tick!\n");
        }
    }
    void stop_cb() { log("stop callback\n"); }
private:
    int tick_interval;
    WvLog log;
};

static MyWvDaemon d;

int main(int argc, char *argv[])
{
    return d.run(argc, argv);
}
