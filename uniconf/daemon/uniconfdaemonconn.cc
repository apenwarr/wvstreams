
#include "uniconfdaemonconn.h"
#include "uniconfdaemon.h"
#include "uniconfiter.h"
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

    WvString *line = gettclline();
    WvString *cmd = NULL;
    WvString *key = NULL;
    while (line)
    {
        WvDynamicBuffer fromline;
        fromline.put(*line, line->len());

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
                WvString *response;
                if (!!source->mainconf.get(*key))
                    response = new WvString("RETN %s %s\n", wvtcl_escape(*key), wvtcl_escape(source->mainconf.get(*key)));
                else
                    response = new WvString("RETN %s \\0\n", wvtcl_escape(*key));

                print(*response);
                delete cmd;
                source->events.add(wvcallback(UniConfCallback, *source, UniConfDaemon::keychanged), this, *key);
                keys.append(key, false);
                cmd = key = 0;
//                wvcon->print(WvString("RETURNING %s.\n", *response));
            }
            else if (*cmd == "subt") // return the subtree(s) of this key
            {
                UniConf *nerf = &source->mainconf[*key];
                if (nerf)
                {
                    WvString send("SUBT %s ", wvtcl_escape(*key));
                    UniConf::Iter i(*nerf);
                    for (i.rewind(); i.next();)
                    {
                        send.append("{%s %s} ", wvtcl_escape(i->name), wvtcl_escape(*i));
                    }
                    send.append("\n");
                    print(send);
//                    wvcon->print(WvString("RETURNING:  %s.\n", send));
                }
                else
                {
                    print(WvString("SUBT %s\n", *key));
//                    wvcon->print(WvString("RETURNING:  SUBT %s\n", *key));
                }
                    
           }
            else if (*cmd == "set") // set the specified value
            {
                WvString *newvalue = wvtcl_getword(fromline);
                source->mainconf[*key] = wvtcl_unescape(*newvalue);
                delete cmd;
                cmd = key = 0;
                source->keymodified = true;
//                wvcon->print(WvString("SET %s TO %s.\n", *key, *newvalue));
            }

            // get a new command & key
            cmd = wvtcl_getword(fromline);
            key = wvtcl_getword(fromline);
            
            // We don't need to unget here, since if we broke on a \n,
            // that means that we were at the end of a word, and since all
            // requests are "single line" via tclstrings, no worries.
        }
        line = gettclline(); 
    }

}


