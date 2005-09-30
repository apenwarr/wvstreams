#include <wvstreamsdaemon.h>
#include <wvtest.h>
#include <wvfork.h>
#include <wvunixsocket.h>

//Callback for the accepted client conection at the server
void client_cb(WvStream &stream, void *)
{
    // Echo everything back
    const char *line = stream.getline();
    if (line)
        stream.print("Client said: %s\n", line);
}     

//Callback function for the daemon
void startup(WvStreamsDaemon &daemon, void *)
{
    WvUnixListener *listener = new WvUnixListener("/tmp/faked", 0700);
    listener->auto_accept(&WvIStreamList::globallist, client_cb); 
    daemon.add_die_stream(listener, true, "Listener");
}

WVTEST_MAIN("Checking Daemon created")
{
    int numTries = 0; 

    //Forking the server (daemon) and client processes
    pid_t child = wvfork();

    //This is for server process
    if(child == 0)
    {
        wvout->print("Running code for server\n");
        WvStreamsDaemon daemon("Sample Daemon", "0.1", startup);
	daemon.pid_file = "/tmp/faked.pid";
	int fake_argc = 2;
	char *fake_argv[] = { "faked", "-d", NULL };
	_exit(daemon.run(fake_argc, fake_argv));
    }

    //This is for client process
    else
    {
        wvout->print("Running code for client\n");
        WvUnixConn *client = NULL;
        
        //Will wait for 10 sec at max for the Daemon to load
        for (int i = 0; i < 10 ; i++)
        { 
            printf("Trying to connect %d \n", numTries);
            numTries++;
            client = new WvUnixConn("/tmp/faked");
	    if (client->isok())
	    {
		printf("Connected!\n");
                break;
	    }
	    WVRELEASE(client);
	    client = NULL;
            sleep(1);
        }

        WVPASS(client != NULL);
	if (client != NULL)
	{
	    client->print("hello\n");
	    const char *line = client->blocking_getline(10000);
	    wvout->print("Server said: %s\n", line);
	    WVPASS(line != NULL && strcmp(line, "Client said: hello") == 0);
	    WVRELEASE(client);
	}
    }
}
