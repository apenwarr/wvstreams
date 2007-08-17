#include "wvlogstream.h"


WvLogStream::WvLogStream(IWvStream *s, WvLog::LogLevel _max_level)
    : WvLogRcv(_max_level)
{
    cloned = s;
}


WvLogStream::~WvLogStream()
{
    WVRELEASE(cloned);
}


void WvLogStream::_mid_line(const char *str, size_t len)
{
    if (cloned)
	cloned->write(str, len);
}
