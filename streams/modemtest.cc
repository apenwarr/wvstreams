/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997, 1998 Worldvisions Computer Technology, Inc.
 */
#include "wvmodem.h"
#include "wvlog.h"
#include "strutils.h"

int main()
{
    WvLog log("modemtest"), modemlog("Rx");
    WvModem modem("/dev/ttyS3", 19200);
    unsigned char buf[1024];
    size_t len;
    bool last_carrier = false, carrier;
    
    WvStreamList l;
    l.append(wvcon, false);
    l.append(&modem, false);
    
    while (modem.isok() && wvcon->isok())
    {
	WvStream *s = l.select_one(100);
	
	carrier = modem.carrier();
	if (last_carrier != carrier)
	{
	    log("Modem %s\n", carrier ? "CONNECTED" : "DISCONNECTED" );
	    last_carrier = carrier;
	}
	
	if (s==wvcon)
	{
	    len = wvcon->read(buf, sizeof(buf));
	    replace_char(buf, '\n', '\r', len);
	    modem.write(buf, len);
	}
	else if (s==&modem)
	{
	    len = modem.read(buf, sizeof(buf));
	    modemlog.write(buf, len);
	}
    }
    
    if (!modem.isok() && modem.geterr())
	log(WvLog::Error, "modem device: %s\n", modem.errstr());
    
    return 0;
}
