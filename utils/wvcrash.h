/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2001 Net Integration Technologies, Inc.
 * 
 * Routines to generate a stack backtrace automatically when a program
 * crashes.
 */
#ifndef __WVCRASH_H
#define __WVCRASH_H

void wvcrash_setup(const char *_argv0);
void wvcrash(int sig);

#endif // __WVCRASH_H
