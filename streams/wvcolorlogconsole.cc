/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * A version of WvColorLogConsole that colorizes the output
 */

#include "wvcolorlogconsole.h"

#include <termios.h>

bool WvColorLogConsole::is_tty(int fd)
{
    struct termios termios;
    return tcgetattr(fd, &termios) == 0;
}


bool WvColorLogConsole::can_colorize(int fd, const char *TERM)
{
    return is_tty(fd)
            && TERM != NULL
            && (strcmp(TERM, "linux") == 0
                    || strcmp(TERM, "ansi") == 0
                    || strcmp(TERM, "xterm") == 0
                    || strcmp(TERM, "rxvt") == 0);
}


WvColorLogConsole::WvColorLogConsole(int _fd, WvLog::LogLevel _max_level) :
    WvLogConsole(_fd, _max_level),
    colorize(WvColorLogConsole::can_colorize(_fd, getenv("TERM")))
{
}


WvColorLogConsole::~WvColorLogConsole()
{
}


void WvColorLogConsole::_begin_line()
{
    if (colorize)
    {
        const char *seq = WvColorLogConsole::color_start_seq(last_level);
        uwrite(seq, strlen(seq));
    }
    WvLogConsole::_begin_line();
    if (colorize)
    {
        const char *seq;
        seq = WvColorLogConsole::clear_to_eol_seq(last_level);
        uwrite(seq, strlen(seq));
        seq = WvColorLogConsole::color_end_seq(last_level);
        uwrite(seq, strlen(seq));
    }
}


void WvColorLogConsole::_mid_line(const char *str, size_t len)
{
    if (colorize)
    {
        const char *seq;
        seq = WvColorLogConsole::color_start_seq(last_level);
        uwrite(seq, strlen(seq));
    }
    WvLogConsole::_mid_line(str, len);
    if (colorize)
    {
        const char *seq;
        seq = WvColorLogConsole::clear_to_eol_seq(last_level);
        uwrite(seq, strlen(seq));
        seq = WvColorLogConsole::color_end_seq(last_level);
        uwrite(seq, strlen(seq));
    }
}


void WvColorLogConsole::_end_line()
{
    if (colorize)
    {
        const char *seq;
        seq = WvColorLogConsole::color_start_seq(last_level);
        uwrite(seq, strlen(seq));
        seq = WvColorLogConsole::clear_to_eol_seq(last_level);
        uwrite(seq, strlen(seq));
        seq = WvColorLogConsole::color_end_seq(last_level);
        uwrite(seq, strlen(seq));
    }
    WvLogConsole::_end_line();
}


const char *WvColorLogConsole::color_start_seq(WvLog::LogLevel log_level)
{
    if (int(log_level) <= int(WvLog::Error))
        return "\e[41;37;1m";
    else if (int(log_level) <= int(WvLog::Warning))
        return "\e[43;37;1m";
    else
        return "\e[40;37;1m";
}


const char *WvColorLogConsole::clear_to_eol_seq(WvLog::LogLevel log_level)
{
    return "\e[0K";
}


const char *WvColorLogConsole::color_end_seq(WvLog::LogLevel log_level)
{
    return "\e[0m";
}
