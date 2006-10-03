/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * A server stream for WvStreamsDebugger connections
 */ 
#ifndef __WVSTREAMSDEBUGGERSERVER_H
#define __WVSTREAMSDEBUGGERSERVER_H

#include "wvstream.h"
#include "wvstreamclone.h"
#include "wvaddr.h"
#include "wvistreamlist.h"
#include "wvstreamsdebugger.h"
#include "wvlog.h"

class WvUnixListener;
class WvTCPListener;

class WvStreamsDebuggerServer : public WvStream
{
    struct Connection : public WvStreamClone
    {
        WvStreamsDebugger debugger;
        WvString salt;
        
        Connection(WvStream *s);
        
        void result_cb(WvStringParm cmd, WvStringList &results);
        
        void send(WvStringParm code, WvStringParm result);
        void send(WvStringParm code, WvStringList &results);
        
        void choose_salt();
    };
    // Username, salt, md5sum("%s%s", salt, password)
    typedef WvCallback<bool, WvStringParm, WvStringParm, WvStringParm> AuthCallback;

    WvLog log;
        
    WvUnixListener *unix_listener;
    void unix_listener_cb(WvStream &, void *);
    void unix_listener_close_cb(WvStream &);
    
    WvTCPListener *tcp_listener;
    void tcp_listener_cb(WvStream &, void *);
    void tcp_listener_close_cb(WvStream &);

    AuthCallback auth_cb;

    void auth_request_cb(WvStream &_s, void *);
    void auth_response_cb(WvStream &_s, void *);
    void ready_cb(WvStream &_s, void *);
    void close_cb(WvStream &_s);
    
    WvIStreamList streams;

public:

    WvStreamsDebuggerServer(WvUnixAddr unix_addr,
            AuthCallback _auth_cb = AuthCallback(),
            WvIPPortAddr tcp_addr = WvIPPortAddr());
    ~WvStreamsDebuggerServer();
            
    void set_auth_callback(AuthCallback _auth_cb)
        { auth_cb = _auth_cb; }
};

#endif // __WVSTREAMSDEBUGGERSERVER_H
