
#include "uniconfdaemonconn.h"
#include "uniconfdaemon.h"
#include "wvtclstring.h"

UniConfDaemonConn::UniConfDaemonConn(WvStream *_s, UniConfDaemon *_source)
    : UniConfConn(_s), source(_source)
{
}

UniConfDaemonConn::~UniConfDaemonConn()
{
    WvStringList::Iter i(keys);
    for (i.rewind(); i.next();)
    {
        const UniConfKey k(*i);
        source->events.del(wvcallback(UniConfCallback, *source, UniConfDaemon::keychanged), this, k);
    }
}

void UniConfDaemonConn::execute()
{
    UniConfConn::execute();
    fillbuffer();

    WvString *line = wvtcl_getword(incomingbuff, "\n");
    WvString *cmd = NULL;
    WvString *key = NULL;
    while (line)
    {
        WvBuffer fromline;
        fromline.put(*line);

        // get the command
        if (!cmd)
            cmd = wvtcl_getword(fromline);
        while(cmd)
        {
            // check the command
            if (*cmd == "quit")
            {
                delete cmd, key;
                cmd = key = 0;
                close();
                return;
            }
            else
                key = wvtcl_getword(fromline);

            if (!key)
                break;

            if (*cmd == "get") // return the specified value
            {
                WvString response("RETN %s %s\n", wvtcl_escape(*key), wvtcl_escape(source->mainconf.get(*key)));
                print(response);
                delete cmd;
                source->events.add(wvcallback(UniConfCallback, *source, UniConfDaemon::keychanged), this, *key);
                keys.append(key, false);
                cmd = key = 0;
            }
            else if (*cmd == "set") // set the specified value
            {
                WvString *newvalue = wvtcl_getword(fromline);
                source->mainconf[*key] = *newvalue;
                delete cmd;
                cmd = key = 0;
                source->keymodified = true;
            }

            // get a new command & key
            cmd = wvtcl_getword(fromline);
            key = wvtcl_getword(fromline);
            
            // We don't need to unget here, since if we broke on a \n,
            // that means that we were at the end of a word, and since all
            // requests are "single line" via tclstrings, no worries.
        }
        line = wvtcl_getword(incomingbuff, "\n");
    }

}


