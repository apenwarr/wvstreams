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
    return new UniConfConn(new WvUnixConn(addr));
}
