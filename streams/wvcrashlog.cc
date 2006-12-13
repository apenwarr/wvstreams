/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * A "Log Receiver" that puts the messages in the wvcrash_ring_buffer
 */
#include "wvcrashlog.h"
#include "wvcrash.h"

WvCrashLog::WvCrashLog(WvLog::LogLevel _max_level) :
    WvLogRcv(_max_level)
{
}


void WvCrashLog::_mid_line(const char *str, size_t len)
{
    wvcrash_ring_buffer_put(str, len);
}


void WvCrashLog::_make_prefix(time_t timenow)
{
    prefix = WvString("%s<%s>: ", last_source, loglevels[last_level]);
    prelen = prefix.len();
}
