/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997, 1998 Worldvisions Computer Technology, Inc.
 *
 * Definition of the WvModem class.  Inherits from WvFile, but does various
 * important details related to modems, like setting baud rates and dropping
 * DTR and the like.
 *
 * Created:	29  Oct 1997		D. Coombs
 *
 */

#ifndef __WVMODEM_H
#define __WVMODEM_H

#include "wvlockfile.h"
#include "wvstream.h"
#include <termios.h>

class WvModem : public WvFile
{
private:
    WvLockFile		lock;
    struct termios	t, old_t;
    int			baud;
    bool		closing;

    void setup_modem();
    int getstatus();
    
public:
    bool die_fast;
    
    WvModem( const char * filename, int _baud );
    virtual ~WvModem();
    
    void hangup();
    virtual void close();
    
    bool carrier();
    
    void speed(int _baud);
    int speed() const
	{ return baud; }
};

#endif
