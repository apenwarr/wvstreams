
#include "uniconfdaemonconn.h"
#include "uniconfdaemon.h"
#include "uniconfiter.h"
#include "wvtclstring.h"


UniConfDaemonConn::UniConfDaemonConn(WvStream *_s, UniConfDaemon *_source)
    : UniConfConn(_s), 
      log(WvString("UniConf to %s", *_s->src()), WvLog::Debug)
{
    print("HELLO UniConf Server ready\n");
    source = _source;
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
    WvString line, cmd;
    
    UniConfConn::execute();
    fillbuffer();

    while (!(line = gettclline()).isnull())
    {
        WvConstStringBuffer fromline(line);

	log(WvLog::Debug5, "Got command: '%s'\n", line);

        while (!(cmd = wvtcl_getword(fromline)).isnull())
        {
            // check the command
	    if (cmd == "help")
	    {
	    	print("OK I know how to: help, get, subt, quit\n");
		return;	    
	    }
            if (cmd == "quit")
            {
	        print("OK quit <null>\n");
                close();
                return;
            }
            WvString key = wvtcl_getword(fromline);
            if (key.isnull())
                break;

            if (cmd == "get") // return the specified value
            {
                print("OK %s %s\n", cmd, key);
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
                print("OK %s %s\n", cmd, key);
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
                print("OK %s %s\n", cmd, key);
                WvString newvalue = wvtcl_getword(fromline);
                source->mainconf[key] = wvtcl_unescape(newvalue);
                source->keymodified = true;
//                wvcon->print(WvString("SET %s TO %s.\n", key, newvalue));
            }
        }
    }
}
