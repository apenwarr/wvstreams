/* -*- Mode: C++ -*-
 *
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */

#ifndef __WVFORK_H
#define __WVFORK_H

#include <unistd.h>
#include "wvhashtable.h"

DeclareWvTable3( int, WvIntTable, );

/**
 * wvfork_start is just like fork, except that it will block the
 * parent until the child process closes the waitfd, to avoid race
 * conditions.
 *
 * For example, wvfork uses it, closing the waitfd only when it is
 * done closing the close-on-exec file descriptors.
 */
extern pid_t wvfork_start(int *waitfd);

/**
 * wvfork() just runs fork(), but it closes all file descriptors that
 * are flagged close-on-exec, since we don't necessarily always run
 * exec() after we fork()...
 *
 * This fixes the year-old mystery bug where WvTapeBackup caused
 * watchdog reboots because the CHILD process wasn't touching it, and
 * it was already open before the fork()...
 */
extern pid_t wvfork( int dontclose1=-1, int dontclose2=-1 );
extern pid_t wvfork( WvIntTable& dontclose );

#endif
