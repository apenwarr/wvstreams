/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 2005 Net Integration Technologies, Inc.
 *
 * Helper classes and functions to add more information to WvCrashes.
 */

#include "wvassert.h"

WvCrashWill::WvCrashWill(const char *will)
    : old_will(wvcrash_read_will())
{
    wvcrash_leave_will(will);
}

WvCrashWill::WvCrashWill(WVSTRING_FORMAT_DEFN)
    : old_will(wvcrash_read_will())
{
    // We use a WvFastString here, because it is a temporary.  init()
    // will duplicate the string into a local buffer, so don't you
    // worry.
    wvcrash_leave_will(WvFastString(WVSTRING_FORMAT_CALL));
}

void WvCrashWill::rewrite(const char *will)
{
    // Don't touch old_will.
    wvcrash_leave_will(will);
}

void WvCrashWill::rewrite(WVSTRING_FORMAT_DEFN)
{
    // Again, since wvcrash_leave_will will duplicate the string, we
    // can use a WvFastString.
    rewrite(WvFastString(WVSTRING_FORMAT_CALL));
}

WvCrashWill::~WvCrashWill()
{
    // Put the old will back.
    wvcrash_leave_will(old_will);
}

