#ifndef __UNICONFCONN_H
#define __UNICONFCONN_H

#include "wvstreamclone.h"
#include "wvbuffer.h"

class UniConfConn : public WvStreamClone
{
public:
    UniConfConn(WvStream *_s);
    virtual ~UniConfConn();

    WvString gettclline();
    virtual void fillbuffer();
    virtual void execute();
    virtual bool isok() const;
protected:

    WvDynamicBuffer incomingbuff;
private:
};

#endif // __UNICONFCONN_H
