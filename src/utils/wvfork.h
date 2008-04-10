/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */

#ifndef __WVFORK_H
#define __WVFORK_H

#include <unistd.h>
#include "wvhashtable.h"

DeclareWvTable3( int, WvIntTable, );

/* wvfork() just runs fork(), but it closes all file descriptors that are
 * flagged close-on-exec, since we don't necessarily always run exec() 
 * after we fork()...
 *
 * This fixes the year-old mystery bug where WvTapeBackup caused watchdog
 * reboots because the CHILD process wasn't touching it, and it was already
 * open before the fork()...
 */
extern pid_t wvfork( int dontclose1=-1, int dontclose2=-1 );
extern pid_t wvfork( WvIntTable& dontclose );

#endif
