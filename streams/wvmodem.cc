/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997, 1998, 1999 Worldvisions Computer Technology, Inc.
 *
 * Implementation of the WvModem class. Inherits from WvFile, but
 * handles various important details related to modems, like setting
 * the baud rate, checking carrier detect, and dropping DTR.
 *
 */

#include "wvmodem.h"
#include <sys/ioctl.h>
    	

struct SpeedLookup {
    int baud;
    speed_t speedt;
};


static SpeedLookup speeds[] = {
    {460800, B460800},
    {230400, B230400},
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


WvModem::WvModem(const char * filename, int _baud)
	: lock(filename)
{
    closing = false;
    baud = _baud;
    die_fast = false;
    
    if (!lock.lock())
    {
	errnum = EBUSY;
	return;
    }
    
    // note: if CLOCAL is not set on the modem, open will
    // block until a carrier detect.  Since we have to open the modem to
    // generate a carrier detect, we have a problem.  So we open the modem
    // nonblocking.  It would then be safe to switch to blocking mode,
    // but that is no longer recommended for WvStream.
    open(filename, O_RDWR|O_NONBLOCK|O_NOCTTY);
    
    if (isok())
	setup_modem();
}


WvModem::~WvModem()
{
    close();
}


void WvModem::setup_modem()
/*************************/
{
    if (!isok()) return;

    if (tcgetattr( fd, &t ) || tcgetattr( fd, &old_t ))
    {
	closing = true;
	seterr(errno);
	return;
    }
    
    drain();
    
    // set up the terminal characteristics.
    // see "man tcsetattr" for more information about these options.
    t.c_iflag &= ~(BRKINT | ISTRIP | IUCLC | IXON | IXANY | IXOFF | IMAXBEL);
    t.c_iflag |= (IGNBRK | IGNPAR);
    t.c_oflag &= ~(OLCUC);
    t.c_cflag &= ~(CSIZE | CSTOPB | PARENB | PARODD);
    t.c_cflag |= (CS8 | CREAD | HUPCL | CRTSCTS | CLOCAL);
    t.c_lflag &= ~(ISIG | XCASE | ECHO);
    tcsetattr( fd, TCSANOW, &t );
    
    // make sure we leave the modem in CLOCAL when we exit, so normal user
    // tasks can open the modem without using nonblocking.
    old_t.c_cflag |= CLOCAL;
    
    // Send a few returns to make sure the modem is "good and zonked".
    if (cfgetospeed(&t) != B0)
    {
	for( int i=0; i<5; i++ ) {
	    write( "\r", 1 );
	    usleep( 10 * 1000 );
	}
    }
    
    // Set the baud rate to 0 for half a second to drop DTR...
    cfsetispeed( &t, B0 );
    cfsetospeed( &t, B0 );
    cfmakeraw( &t );
    tcsetattr( fd, TCSANOW, &t );
    if (carrier())
	usleep( 500 * 1000 );
    
    speed(baud);
    usleep( 10 * 1000 );
    
    drain();
}


void WvModem::hangup()
/********************/
{
    int i, oldbaud = baud;
    
    if (die_fast || !isok()) return;

    // politely abort any dial in progress, to avoid locking USR modems.
    // we should only do this if we have received any response from the modem,
    // so that WvModemScan can run faster.
    drain();
    write( "\r", 1 );
    for (i = 0; !select(200) && i < 10; i++)
	write( "\r", 1 );
    drain();

    // drop DTR for a while, if we're still online
    if (carrier())
    {
	cfsetospeed( &t, B0 );
	tcsetattr( fd, TCSANOW, &t );
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


void WvModem::close()
{
    if (fd >= 0)
    {
	if (!closing)
	{
	    closing = true;
	    hangup();
	}
	closing = true;
	tcflush(fd, TCIOFLUSH);
	tcsetattr(fd, TCSANOW, &old_t);
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

    cfsetispeed( &t, B0 ); // auto-match to output speed
    cfsetospeed( &t, s );
    tcsetattr( fd, TCSANOW, &t );

    tcgetattr( fd, &t );
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


int WvModem::getstatus()
{
    if (!isok()) return 0;
    int status = 0;
    ioctl(fd, TIOCMGET, &status);
    return status;
}


bool WvModem::carrier()
{
    return (getstatus() & TIOCM_CD) ? 1 : 0;
}
