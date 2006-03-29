#pragma once

#include "wvstreamclone.h"

class MySocket :
    public WvStreamClone
{
public:
    MySocket(WvStream *_clone);
    void execute();
    ~MySocket(void);

};
