/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997, 1998, 1999 Worldvisions Computer Technology, Inc.
 *   Copyright (C) 1999 Red Hat, Inc.
 *
 * Definition of the WvModemBase and WvModem classes.  Inherit from WvFile,
 * but do various important details related to modems, like setting baud
 * rates and dropping DTR and the like.
 *
 */

#ifndef __WVMODEM_H
#define __WVMODEM_H

#include "wvlockfile.h"
#include "wvstream.h"
#include <termios.h>


// WvModemBase provides the methods used to control a modem, but
// without real implementation for most of them, so that they can
// be used in contexts where modem control is undesirable without
// reimplementing calling code for such uses.
class WvModemBase : public WvFile
{
protected:
    struct termios	t;
    int			baud;

    int get_real_speed();
    WvModemBase() { }

public:
    bool die_fast;
    
    WvModemBase(int _fd);
    virtual ~WvModemBase();
    
    // do-nothing methods that aren't needed in WvModemBase
    virtual void close();
    virtual bool carrier();
    virtual int speed(int _baud);

    // may need to hangup for redial reasons
    virtual void hangup();

    virtual int speed()
	{ return get_real_speed(); }
};


// WvModem implements a named modem that really needs to be opened,
// closed, and manipulated in lots of ways
class WvModem : public WvModemBase
{
private:
    WvLockFile		lock;
    struct termios	old_t;
    bool		closing;

    void setup_modem();
    int getstatus();
    
public:
    WvModem( const char * filename, int _baud );
    virtual ~WvModem();
    
    virtual void close();
    
    bool carrier();
    
    int speed(int _baud);
    int speed()
	{ return baud; }
};

#endif
