/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * wvfork() just runs fork(), but it closes all file descriptors that are
 * flagged close-on-exec, since we don't necessarily always run exec() 
 * after we fork()...
 *
 * This fixes the year-old mystery bug where WvTapeBackup caused watchdog
 * reboots because the CHILD process wasn't touching it, and it was already
 * open before the fork()...
 *
 * If you want to explicitly leave a file descriptor OPEN, even if it's
 * marked close-on-exec, then add the fd number to dontclose, and pass that
 * to wvfork().  This is mainly useful for WvLoopbacks -- you may want
 * certain ones open or closed depending on which call to wvfork() you're
 * making.  (for WvTapeBackup, you want the three backup loopbacks open,
 * and, say, any WvResolver loopbacks closed.)
 */

#include <fcntl.h>

#include "wvfork.h"

#define MAX_FD  256     // FIXME: is this enough?

pid_t wvfork( int dontclose1, int dontclose2 )
/********************************************/
{
    WvIntTable t( 1 );
    if( dontclose1 >= 0 )
        t.add( &dontclose1, false );
    if( dontclose2 >= 0 )
        t.add( &dontclose2, false );
    return( wvfork( t ) );
}

pid_t wvfork( WvIntTable& dontclose )
/***********************************/
{
    pid_t pid = fork();

    if( pid != 0 ) {
        // parent process, or error
        return( pid );
    }

    // child process
    // check the close-on-exec flag of all file descriptors
    for( int fd=0; fd<MAX_FD; fd++ ) {
        if( !dontclose[ fd ] && ( fcntl( fd, F_GETFD ) & FD_CLOEXEC ) > 0 )
            close( fd );
    }
    return( pid );
}

