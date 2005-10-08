/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Routines to generate a stack backtrace automatically when a program
 * crashes.
 */
#ifndef __WVCRASH_H
#define __WVCRASH_H

#include <wvcallback.h>

typedef WvCallback<void, int> WvCrashCallback;

void wvcrash_setup(const char *_argv0, const char *_desc = 0);
void wvcrash(int sig);
void wvcrash_add_signal(int sig);
WvCrashCallback wvcrash_set_callback(WvCrashCallback callback);

// Leave a last will and testament in the WvCrash, if your program dies.
void wvcrash_leave_will(const char *will);
// Read the will back.
const char *wvcrash_read_will();

#endif // __WVCRASH_H
