
#ifndef __WV_UCONFD_H
#define __WV_UCONFD_H

#include <strutils.h>
#include <signal.h>
#include <netinet/in.h>

//#include "uniconfconn.h"
#include "uniconfini.h"
#include "uniconf.h"
#include "wvdiriter.h"
#include "wvfile.h"
#include "wvtclstring.h"
#include <wvstreamlist.h>
#include <wvstreamclone.h>
#include <wvlog.h>
#include <wvunixsocket.h>
#include <wvaddr.h>

class UniConfDaemon
{
public:
    UniConfDaemon();
    ~UniConfDaemon();
    UniConf *domount(WvString mode, WvString file, WvString mp);
    void run();
    void addstream(WvStream *s);

//    static const int OPCODE_LENGTH;
    bool want_to_die;
    WvLog log;
    UniConf mainconf;
protected:
    void handlerequest(WvStream &s, void *userdata);
private:
    WvStreamList *l;
    static const WvString DEFAULT_CONFIG_FILE;
};

/*class UniConfDConn : public UniConfConn
{
public:
    UniConfDConn(WvStream *s, UniConfDaemon *owner);
    ~UniConfDConn();
//    bool doread(char *buffer, long length, int nextsize=0);
//    bool doread(long *size, int nextsize=0);
protected:
    virtual void execute();
private:
    UniConfDaemon *owner;
};
*/
#endif
