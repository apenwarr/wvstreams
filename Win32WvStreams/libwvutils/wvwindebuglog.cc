#include "wvwindebuglog.h"
#include "wvlog.h"

void WvWinDebugLog::_mid_line(const char *str, size_t len)
{
    ::OutputDebugString(str);
}

WvWinDebugLog::~WvWinDebugLog()
{
    end_line();
}
