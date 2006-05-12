/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *   Copyright (C) 1999 Red Hat, Inc.
 *
 * Implementation of the WvModem class. Inherits from WvFile, but
 * handles various important details related to modems, like setting
 * the baud rate, checking carrier detect, and dropping DTR.
 *
 */

#include "wvmodem.h"
#include <sys/ioctl.h>

#if HAVE_LINUX_SERIAL_H
# include <linux/serial.h>
#endif

#if ! HAVE_CFMAKERAW
static inline void cfmakeraw(struct termios *termios_p)
{
    termios_p->c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
    termios_p->c_oflag &= ~OPOST;
    termios_p->c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
    termios_p->c_cflag &= ~(CSIZE|PARENB);
    termios_p->c_cflag |= CS8;
}
#endif

struct SpeedLookup {
    int baud;
    speed_t speedt;
};


static SpeedLookup speeds[] = {
#ifdef B460800
    {460800, B460800},
#endif
#ifdef B230400
    {230400, B230400},
#endif
    {115200, B115200},
    { 57600, B57600},
    { 38400, B38400},
    { 19200, B19200},
    {  9600, B9600},
    {  4800, B4800},
    {  2400, B2400},
    {  1200, B1200},
    {   300, B300}
};


WvModemBase::WvModemBase(int _fd) : WvFile(_fd)
{
    get_real_speed();
}


WvModemBase::~WvModemBase()
{
    // nothing needed
}


int WvModemBase::get_real_speed()
{
    speed_t s;
    
    if (!isok()) return 0;

    tcgetattr( getrfd(), &t );
    s = cfgetospeed( &t );
    for (unsigned int i = 0; i < sizeof(speeds) / sizeof(*speeds); i++)
    {
	if (speeds[i].speedt == s)
	{
	    baud = speeds[i].baud;
	    break;
	}
    }

    return baud;
}


void WvModemBase::close()
{
    // no file open, no need to close it
}


bool WvModemBase::carrier()
{
    return true;
}


int WvModemBase::speed(int)
{
    return baud;
}


void WvModemBase::hangup()
{
    int i, oldbaud = baud;
    
    if (die_fast || !isok()) return;

    // politely abort any dial in progress, to avoid locking USR modems.
    // we should only do this if we have received any response from the modem,
    // so that WvModemScan can run faster.
    drain();
    write( "\r", 1 );
    // FIXME: should be iswritable, but based on the numer of msec params
    // tossed around I assume modems are very timing-sensitive
    for (i = 0; !select(200, false, true) && i < 10; i++)
	write( "\r", 1 );
    drain();

    // drop DTR for a while, if still online
    if (carrier())
    {
	cfsetospeed( &t, B0 );
	tcsetattr( getrfd(), TCSANOW, &t );
	for (i = 0; carrier() && i < 10; i++)
	    usleep( 100 * 1000 );

	// raise DTR again, restoring the old baud rate
	speed(oldbaud);
    }
    
    if (carrier())
    {
	// need to do +++ manual-disconnect stuff
	write( "+++", 3 );
	usleep( 1500 * 1000 );
	write( "ATH\r", 4 );
	
	for (i = 0; carrier() && i < 5; i++)
	    usleep( 100 * 1000 );
    }
}



WvModem::WvModem(WvStringParm filename, int _baud, bool rtscts, bool _no_reset)
    : WvModemBase(), lock(filename), log("WvModem", WvLog::Debug1)
{
    closing = false;
    baud = _baud;
    die_fast = false;
    no_reset = _no_reset;
    have_old_t = false;
    
    if (!lock.lock())
    {
	seterr(EBUSY);
	return;
    }
    
    // note: if CLOCAL is not set on the modem, open will
    // block until a carrier detect.  Since we have to open the modem to
    // generate a carrier detect, we have a problem.  So we open the modem
    // nonblocking.  It would then be safe to switch to blocking mode,
    // but that is no longer recommended for WvStream.
    open(filename, O_RDWR|O_NONBLOCK|O_NOCTTY);
    
    if (isok())
	setup_modem(rtscts);
}


WvModem::~WvModem()
{
    close();
}


void WvModem::setup_modem(bool rtscts)
{
    if (!isok()) return;

    if (tcgetattr(getrfd(), &t) || tcgetattr(getrfd(), &old_t))
    {
	closing = true;
	seterr(errno);
	return;
    }
    have_old_t = true;
    
    drain();
    
#if HAVE_LINUX_SERIAL_H
    struct serial_struct old_sinfo, sinfo;
    sinfo.reserved_char[0] = 0;
    if (ioctl(getrfd(), TIOCGSERIAL, &old_sinfo) < 0)
	log("Cannot get information for serial port.");
    else
    {
	sinfo = old_sinfo;
	// Why there are two closing wait timeouts, is beyond me
	// but there are... apparently the second one is deprecated
	// but why take a chance...
	sinfo.closing_wait = ASYNC_CLOSING_WAIT_NONE;
	sinfo.closing_wait2 = ASYNC_CLOSING_WAIT_NONE;

	if (ioctl(getrfd(), TIOCSSERIAL, &sinfo) < 0)
	    log("Cannot set information for serial port.");
    }
#endif

    // set up the terminal characteristics.
    // see "man tcsetattr" for more information about these options.
    t.c_iflag &= ~(BRKINT | ISTRIP | IUCLC | IXON | IXANY | IXOFF | IMAXBEL);
    t.c_iflag |= (IGNBRK | IGNPAR);
    t.c_oflag &= ~(OLCUC);
    t.c_cflag &= ~(CSIZE | CSTOPB | PARENB | PARODD);
    t.c_cflag |= (CS8 | CREAD | HUPCL | CLOCAL);
    if(rtscts)
        t.c_cflag |= CRTSCTS;
    t.c_lflag &= ~(ISIG | XCASE | ECHO);
    tcsetattr(getrfd(), TCSANOW, &t);
    
    // make sure we leave the modem in CLOCAL when we exit, so normal user
    // tasks can open the modem without using nonblocking.
    old_t.c_cflag |= CLOCAL;
    
    // Send a few returns to make sure the modem is "good and zonked".
    if (cfgetospeed(&t) != B0 && !no_reset)
    {
	for(int i=0; i<5; i++) 
	{
	    write("\r", 1);
	    usleep(10 * 1000);
	}
    }
    
    // Set the baud rate to 0 for half a second to drop DTR...
    cfsetispeed(&t, B0);
    cfsetospeed(&t, B0);
    cfmakeraw(&t);
    tcsetattr(getrfd(), TCSANOW, &t);
    if (carrier())
	usleep(500 * 1000);
    
    speed(baud);
    usleep(10 * 1000);
    
    drain();
}


void WvModem::close()
{
    if (!closed)
    {
	if (!closing)
	{
	    closing = true;
	    if (!no_reset)
		hangup();
	    else
	    {
		drain();
		cfsetospeed(&t, B0);
		// If this works??
		write("\r");
	    }
	}
    
	closing = true;
	if (getrfd() >= 0)
	{
	    tcflush(getrfd(), TCIOFLUSH);
            if (have_old_t)
                tcsetattr(getrfd(), TCSANOW, &old_t);
	}
	WvFile::close();
	closing = false;
    }
}


int WvModem::speed(int _baud)
{
    speed_t s = B0;
    baud = 0;
    for (unsigned int i = 0; i < sizeof(speeds) / sizeof(*speeds); i++)
    {
	if (speeds[i].baud <= _baud)
	{
	    s = speeds[i].speedt;
	    break;
	}
    }

    cfsetispeed(&t, B0); // auto-match to output speed
    cfsetospeed(&t, s);
    tcsetattr(getrfd(), TCSANOW, &t);

    return get_real_speed();
}


int WvModem::getstatus()
{
    if (!isok()) return 0;
    int status = 0;
    ioctl(getrfd(), TIOCMGET, &status);
    return status;
}


bool WvModem::carrier()
{
    return (getstatus() & TIOCM_CD) ? 1 : 0;
}
