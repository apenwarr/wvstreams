#include "uniconfconn.h"
#include "wvtclstring.h"

// CONSTANTS FOR CONTROL CONNECTIONS
const WvString UniConfConn::UNICONF_GET("get");
const WvString UniConfConn::UNICONF_SET("set");
const WvString UniConfConn::UNICONF_DEL("del");
const WvString UniConfConn::UNICONF_SUBTREE("subt");
const WvString UniConfConn::UNICONF_RECURSIVESUBTREE("rsub");
const WvString UniConfConn::UNICONF_REGISTER("reg");
    
const WvString UniConfConn::UNICONF_RETURN("RETN");
const WvString UniConfConn::UNICONF_FORGET("FGET");
const WvString UniConfConn::UNICONF_SUBTREE_RETURN("SUBT");
const WvString UniConfConn::UNICONF_OK("OK");
const WvString UniConfConn::UNICONF_FAIL("FAIL");

const WvString UniConfConn::UNICONF_QUIT("quit");
const WvString UniConfConn::UNICONF_HELP("help");

UniConfConn::UniConfConn(WvStream *_s) : WvStreamClone(_s)
{
}

UniConfConn::~UniConfConn()
{
}

WvString UniConfConn::gettclline()
{
    if (select(0,true,false,false))
        fillbuffer();

    return wvtcl_getword(incomingbuff, "\n");
}

// Read data from the incoming stream into our buffer.
void UniConfConn::fillbuffer()
{
    int len = -1;
    char *cptr[1024];
    
    while (len != 0 && select(0, true, false, false))
    {
        len = read(cptr, 1023);
        cptr[len] ='\0';
        incomingbuff.put(cptr, len);
    }
}

void UniConfConn::execute()
{
    WvStreamClone::execute();
    // now do nothing
}

bool UniConfConn::isok() const
{
    return cloned->isok();
}
