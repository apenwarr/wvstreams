#include "uniconfconn.h"
#include "wvtclstring.h"

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

