#include "uniconfconnfactory.h"
#include "wvunixsocket.h"

UniConfUSocketFactory::UniConfUSocketFactory(WvUnixAddr _addr) : addr(_addr)
{
}

UniConfUSocketFactory::~UniConfUSocketFactory()
{
}

UniConfConn *UniConfUSocketFactory::open()
{
    WvUnixConn *conn = new WvUnixConn(addr);
    if (conn->isok())
        return new UniConfConn(conn);
    else
        return NULL;
}
