/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2004 Net Integration Technologies, Inc.
 * 
 * WvStreams implementation of ptys under Linux.
 *
 * For more information on programming ptys, see chapter 19 of
 * Stevens' "Advanced Programming in the UNIX Environment"
 */
#ifndef __WVPTY_H
#define __WVPTY_H

#include "wvfdstream.h"

class WvPty : public WvFDStream
{

    private:

        WvString _master;
        WvString _slave;
        pid_t _pid;
        int _exit_status;

        bool open_master();
        bool open_slave();

        void monitor_child(bool wait);

    public:

        WvPty(const char *program, const char * const *argv);

        void kill(int signum);
        bool child_exited();
        bool child_killed();
        int finish();
        int exit_status();
        
        const char *master() const
            { return _master; }
        const char *slave() const
            { return _slave; }
        pid_t pid() const
            { return _pid; }
};

#endif // __WVPTY_H
