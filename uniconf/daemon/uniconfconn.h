
#ifndef __WV_UCONFCONN_H
#define __WV_UCONFCONN_H

#include <netinet/in.h>
#include "wvstreamclone.h"

class UniConfConn : public WvStreamClone
{
public:
    UniConfConn(WvStream *s);
    ~UniConfConn();
    bool doread(char *buffer, long length, int nextsize=0);
    bool doread(long *size, int nextsize=0);
    static const int OPCODE_LENGTH;
protected:
    virtual void execute();
private:
};


#endif
