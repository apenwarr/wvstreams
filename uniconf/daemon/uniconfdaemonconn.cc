
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
        source->events.del(wvcallback(UniConfCallback, *source,
            UniConfDaemon::keychanged), this, k);
    }
}

void UniConfDaemonConn::execute()
{
    UniConfConn::execute();
    fillbuffer();

    for (;;)
    {
        WvString line = gettclline();
        if (line.isnull())
            break;
        WvConstStringBuffer fromline(line);

        for (;;)
        {
            WvString cmd = wvtcl_getword(fromline);
            if (cmd.isnull())
                break;

            // check the command
            if (cmd == "quit")
            {
                close();
                return;
            }
            WvString key = wvtcl_getword(fromline);
            if (key.isnull())
                break;

            if (cmd == "get") // return the specified value
            {
                WvString response;
                if (!!source->mainconf.get(key))
                    response = WvString("RETN %s %s\n", wvtcl_escape(key),
                        wvtcl_escape(source->mainconf.get(key)));
                else
                    response = WvString("RETN %s \\0\n", wvtcl_escape(key));

                print(response);
                source->events.add(wvcallback(UniConfCallback,
                    *source, UniConfDaemon::keychanged), this, key);
                keys.append(new WvString(key), true);
//                wvcon->print(WvString("RETURNING %s.\n", response));
            }
            else if (cmd == "subt") // return the subtree(s) of this key
            {
                UniConf *nerf = &source->mainconf[key];
                if (nerf)
                {
                    WvString send("SUBT %s ", wvtcl_escape(key));
                    UniConf::Iter i(*nerf);
                    for (i.rewind(); i.next();)
                    {
                        send.append("{%s %s} ", wvtcl_escape(i->name),
                            wvtcl_escape(*i));
                    }
                    send.append("\n");
                    print(send);
//                    wvcon->print(WvString("RETURNING:  %s.\n", send));
                }
                else
                {
                    print(WvString("SUBT %s\n", key));
//                    wvcon->print(WvString("RETURNING:  SUBT %s\n", key));
                }
                    
            }
            else if (cmd == "set") // set the specified value
            {
                WvString newvalue = wvtcl_getword(fromline);
                source->mainconf[key] = wvtcl_unescape(newvalue);
                source->keymodified = true;
//                wvcon->print(WvString("SET %s TO %s.\n", key, newvalue));
            }
        }
    }
}
