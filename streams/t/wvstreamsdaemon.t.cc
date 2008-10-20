#include <wvstreamsdaemon.h>
#include <wvtest.h>
#include <wvfork.h>
#include <wvfile.h>
#include <wvunixsocket.h>
#include <wvunixlistener.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <wvtimeutils.h>

// This needs to be global (and not part of a WvStreamsDaemonTester class)
// because the WvStreamsDaemon startup callback below needs to know the name
// of the socket to start on.
static WvString sock_name;
static WvString pidfile;

void init()
{
    sock_name = WvString("/tmp/WvStreamsDaemon_test_sock-%s", getpid());
    pidfile = WvString("/tmp/WvStreamsDaemon_test_sock-%s.pid", getpid());
}

//Callback for the accepted client conection at the server
static void client_cb(WvStream &stream)
{
    // Echo everything back
    const char *line = stream.getline();
    if (line)
        stream.print("Client said: %s\n", line);
}     


static void accept_cb(IWvStream *_conn)
{
    WvUnixConn *conn = (WvUnixConn*)_conn;
    conn->setcallback(wv::bind(client_cb, wv::ref(*conn)));
    WvIStreamList::globallist.append(conn, true, "WvUnixConn");
}


//Callback function for the daemon
static void startup(WvStreamsDaemon *daemon)
{
    signal(SIGPIPE, SIG_IGN);
    WvUnixListener *listener = new WvUnixListener(sock_name, 0700);
    listener->onaccept(wv::bind(accept_cb, _1)); 
    daemon->add_die_stream(listener, true, "Listener");
}

// Returns a new WvUnixConn connected to name.  Returns NULL if it could not
// connect.  If expect_connect is false, will retry until it cannot connect
// (up to num_retries times).
WvUnixConn *connect_to_daemon(WvString name, int num_retries, 
        bool expect_connect)
{
    WvUnixConn *conn = NULL;
    for (int i = 0; i < num_retries ; i++)
    { 
        WVFAILEQ(WvString("Trying to connect %s \n", i).cstr(), "");
        conn = new WvUnixConn(name);
        bool is_connected = conn->isok();
        if (is_connected && expect_connect)
        {
            WVPASS("Connected!\n");
            break;
        }
        WVRELEASE(conn);
        conn = NULL;

        if (!is_connected && !expect_connect)
        {
            WVPASS("Connection unavailable.");
            break;
        }
        wvdelay(100);
    }
    return conn;
}

void wait_for_child(pid_t child)
{
    int status = 0;
    int count = 0;
    int max_tries = 10;
    pid_t rv;
    do
        rv = waitpid(child, &status, 0);
    while (rv == -1 && errno == EINTR && ++count < max_tries);
    // Print out any error we received, to aid debugging
    WVPASSEQ(errno, errno);
    WVFAILEQ(count, max_tries);
    WVPASSEQ(rv, child);
    WVPASS(WIFEXITED(status));
    WVPASSEQ(WEXITSTATUS(status), 0);
}

WVTEST_MAIN("Checking Daemon created")
{
    init();

    //Forking the server (daemon) and client processes
    pid_t child = wvfork();

    if (child == 0)
    {
        // This is the server process
        WvStreamsDaemon *daemon = NULL;
        wvout->print("Running code for server\n");
        daemon = new WvStreamsDaemon("Sample Daemon", "0.1",
				     wv::bind(startup, wv::ref(daemon)));
	daemon->pid_file = pidfile;
	int fake_argc = 2;
	char *fake_argv[] = 
            { (char*)"WvStreamsDaemon_nonexistant", (char*)"-d", NULL };
	_exit(daemon->run(fake_argc, fake_argv));
    }
    else
    {
        // This is the client process
        wvout->print("Running code for client\n");
                
        // Will wait for 10 sec at max for the daemon to load
        WvUnixConn *client = connect_to_daemon(sock_name, 60, true);

	if (WVPASS(client != NULL))
	{
	    client->print("hello\n");
	    const char *line = client->blocking_getline(10000);
	    wvout->print("Server said: %s\n", line);
	    if (WVPASS(line != NULL)) 
                WVPASSEQ(WvString(line), WvString("Client said: hello"));
	    WVRELEASE(client);
	    client = NULL;
	}

        pid_t pid_daemon = -1;   
 
        WvFile has_pid(pidfile, O_RDONLY);
        if(WVPASS(has_pid.isok()))
        {
            char *line = has_pid.getline(0);
            if (WVPASS(line))
                pid_daemon = atoi(line);
        }
      
        printf("Process id for daemon is %d\n", pid_daemon);
        if (WVPASS(pid_daemon > 0))
        {
            kill(pid_daemon, SIGTERM);
            // The daemon won't be available for us to wait on, since it's
            // done a ForkTwiceSoTheStupidThingWorksRight, and its parent (our
            // child) has exited anyway.  Still, let's attempt to ensure
            // it gets the signal before continuing.
            pid_t rv;
            do 
                rv = waitpid(pid_daemon, NULL, 0);
            while (rv == -1 && errno == EINTR);
            WVPASSEQ(errno, ECHILD);
            WVPASSEQ(rv, -1);
        }

        // Clean up our child
        wait_for_child(child);
  
        // connect_to_daemon returns NULL if it couldn't connect (i.e. if the
        // stream is not isok()); this is what we expect.
        WvUnixConn *check_conn = connect_to_daemon(sock_name, 60, false);
        WVFAIL(check_conn);
        WVRELEASE(check_conn);
    }
}


