#include <wvstreamsdaemon.h>
#include <wvtest.h>
#include <wvfork.h>
#include <wvfile.h>
#include <wvunixsocket.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>

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

#if 0
WVTEST_MAIN("Checking Daemon created")
{
    //Forking the server (daemon) and client processes
    pid_t child = wvfork();

    WvStreamsDaemon *daemon= NULL;
    WvUnixConn *client = NULL;
    
    //This is for server process
    if(child == 0)
    {
        wvout->print("Running code for server\n");
        daemon= new WvStreamsDaemon("Sample Daemon", "0.1", startup);
	daemon->pid_file = "/tmp/faked.pid";
	int fake_argc = 2;
	char *fake_argv[] = { "faked", "-d", NULL };
	_exit(daemon->run(fake_argc, fake_argv));
    }

    //This is for client process
    else
    {
        wvout->print("Running code for client\n");
                
        //Will wait for 10 sec at max for the Daemon to load
        for (int i = 0; i < 10 ; i++)
        { 
            printf("Trying to connect %d \n", i);
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

        pid_t pid_daemon = -1;   
 
        WvFile *has_pid = new WvFile("/tmp/faked.pid", O_RDONLY);

        if(has_pid->isok())
        {
            char *line = has_pid->getline(0);
            pid_daemon = atoi(line);
            WVRELEASE(has_pid);
        }
      
        printf("%d\n", pid_daemon);
      
        kill (pid_daemon, SIGTERM);
  
        for (int i = 0; i < 10 ; i++)
        { 
            printf("Trying to connect %d \n", i);
            client = new WvUnixConn("/tmp/faked");
            if (!client->isok())
            {
                printf("Disconnected! and Daemon Process Killed\n");
                break;
	    }
            else printf("Still connecting\n");
	
            WVRELEASE(client);
	    client = NULL;
            sleep(1);
        }
    
    }
}
#endif

