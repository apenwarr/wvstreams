/*
 * Worldvisions Weaver Software
 *   Copyright (C) 1997 - 2002 Net Integration Technologies Inc.
 *
 * Daemon program for the uniconf configuration system.
 */

#include <wvtclstring.h>
#include <wvtcp.h>
#include "uniconfdaemon.h"

const WvString UniConfDaemon::DEFAULT_CONFIG_FILE = "uniconf.ini";
//const int UniConfDaemon::OPCODE_LENGTH = 4;

// Daemon Connection
/*UniConfDConn::UniConfDConn(WvStream *s, UniConfDaemon *parent) : UniConfConn(s), owner(parent)
{
    uses_continue_select = true;
}

UniConfDConn::~UniConfDConn()
{
    // end our continue_select here
    terminate_continue_select();
    close();
}

void UniConfDConn::execute()
{
    UniConfConn::execute();
    
    char opcode[UniConfConn::OPCODE_LENGTH+1];
    long stringsize = -1;

    // FIXME: The do read function returns false if the stream craps out on us
    // in the middle of a read attempt.  Needed to prevent the daemon from
    // locking up if a stream dies.  I have a gut feeling there's a better
    // way to do this, but I'm not sure how yet.
    if (!doread(opcode, UniConfConn::OPCODE_LENGTH, sizeof(stringsize))
     || !doread(&stringsize))
        return;
    char key[stringsize+1];
    if (!doread(key, stringsize, UniConfConn::OPCODE_LENGTH))
        return;
    key[stringsize] = '\0';

    if (!strcmp(opcode, "GETK"))
    {
        // Ok get and return the values....
        // SOMEWHERE in here though, we need to set the notify data.
        WvString value("%s",owner->mainconf[key]);
        long size = value.len();
        long tsize = htonl(size);
        wvcon->print("Value:  %s\n", value);
        //print("Value: %s\n", value);
        
        //print("RETN%s%s", strlen(owner->mainconf[key]));
        write("RETN");
        write(&tsize, sizeof(size));
        write(value, size);
        queuemin(UniConfConn::OPCODE_LENGTH);
    }
    else if (!strcmp(opcode, "SETK"))
    {
        // get the value to set [key] to.
        queuemin(sizeof(stringsize));
        if (!doread(&stringsize))
            return;

        char value[stringsize+1];

        if (!doread(value, stringsize, UniConfConn::OPCODE_LENGTH))
            return;
        owner->mainconf[key] = value;
    }
}
*/

// Daemon

UniConfDaemon::UniConfDaemon() : 
    want_to_die(false), log("UniConfDaemon")
{
    l = new WvStreamList;
}

UniConfDaemon::~UniConfDaemon()
{
    delete l;
}

// Look after the actual mounting, where mode indicates the type of config file
// we are using, file is the location of the actual file, and mp is the point
// where we want to mount the contents into the config tree.
UniConf *UniConfDaemon::domount(WvString mode, WvString file, WvString mp)
{
    wvcon->print("Attempting to mount the %s file %s to point:  %s.\n",
            mode, file, mp);
    UniConf *mounted = &mainconf[mp];
    if (mode == "ini")
    {
        mounted->generator = new UniConfIniFile(mounted, file);
        mounted->generator->load();
        return mounted;
    }
    else
        return NULL;
}

void UniConfDaemon::handlerequest(WvStream &s, void *userdata)
{
    int len = -1;
    WvBuffer *buf = (WvBuffer *)userdata;
    char *cptr[1024];
    
    while (len != 0 && s.select(0, true, false, false))
    {
        len = s.read(cptr, 1023);
        cptr[len] ='\0';
        buf->put(cptr, len);
    }

    WvString *line = wvtcl_getword(*buf, "\n");
    while (line)
    {
        WvBuffer foo;
        foo.put(*line);

        // get the command
        WvString *cmd = wvtcl_getword(foo);
        WvString *key = wvtcl_getword(foo);
        while(cmd && key)
        {
            // check the command
            if (*cmd == "get") // return the specified value
            {
                WvString response("RETN %s %s\n", wvtcl_escape(*key), wvtcl_escape(mainconf[*key]));
                s.print(response);
            }
            else if (*cmd == "set") // set the specified value
            {
                WvString *newvalue = wvtcl_getword(foo);
                mainconf[*key] = *newvalue;
            }
            else
            {
                wvcon->print("Received unrecognized command:  %s and key: %s.\n", *cmd, *key);
            }

            // get a new command & key
            cmd = wvtcl_getword(foo);
            key = wvtcl_getword(foo);
            
            // We don't need to unget here, since if we broke on a \n,
            // that means that we were at the end of a word, and since all
            // requests are "single line" via tclstrings, no worries.
        }
        line = wvtcl_getword(*buf, "\n");
    }

}

// Add the specified stream to our stream list, after wrapping it
// as a UniConfConn(ection), and setting it's queuemin value to the
// OPCODE_LENGTH
void UniConfDaemon::addstream(WvStream *s)
{
    WvBuffer *newbuf = new WvBuffer;
    s->setcallback(wvcallback(WvStreamCallback, *this, UniConfDaemon::handlerequest), newbuf);
    l->append(s, true);
}

// Daemon looks after running
void UniConfDaemon::run()
{
    WvBuffer *newbuf = new WvBuffer; // new
    // Mount our initial config file.
    domount("ini", DEFAULT_CONFIG_FILE, "/");
//    mainconf.dump(*wvcon);
    // Now listen on our unix socket.
    // but first, make sure that everything was cleaned up nicely before.
    system("mkdir -p /tmp/uniconf");
    system("rm -fr /tmp/uniconf/uniconfsocket");
    WvUnixListener *list = new WvUnixListener(WvUnixAddr("/tmp/uniconf/uniconfsocket"), 0755);
    if (!list->isok())
    {
        wvcon->print("ERROR:  WvUnixListener could not be created.\n");
        wvcon->print("Error Reason:  %s\n", list->errstr());
        exit(2);
    }
    list->auto_accept(l, wvcallback(WvStreamCallback, *this, UniConfDaemon::handlerequest), newbuf); // new
    l->append(list, true); // new

    // Now listen on the correct TCP port
    WvTCPListener *tlist = new WvTCPListener(WvIPPortAddr("0.0.0.0", 4111));
    if (!tlist->isok())
    {
        wvcon->print("ERROR:  WvTCPListener could not be created.\n");
        wvcon->print("Error Reason:  %s\n", tlist->errstr());
        exit(2);
    }
    tlist->auto_accept(l, wvcallback(WvStreamCallback, *this, UniConfDaemon::handlerequest), newbuf); // new
    l->append(tlist, true);
    wvcon->print("RUNNING\n");
    while (!want_to_die)
    {
        if(l->select(250, true, false))
            l->callback();
    }
    list->close();
    tlist->close();
    if (l) delete l;
}
