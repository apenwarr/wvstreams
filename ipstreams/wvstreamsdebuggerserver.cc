/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 */
#include "wvstreamsdebuggerserver.h"
#include "wvunixsocket.h"
#include "wvtcp.h"

void WvStreamsDebuggerServer::Connection::choose_salt()
{
    const int salt_size = 8;
    const int salt_alphabet_size = 26+26+10;
    const char salt_chars[salt_alphabet_size+1] =
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    
    salt.setsize(salt_size+1);
    for (int i=0; i<salt_size; ++i)
        salt.edit()[i] = salt_chars[rand() % salt_alphabet_size];
    salt.edit()[salt_size] = '\0';
}


WvStreamsDebuggerServer::Connection::Connection(WvStream *s) :
    WvStreamClone(s)
{
}


void WvStreamsDebuggerServer::Connection::result_cb(WvStringParm,
        WvStringList &results)
{
    send("-", results);
}


void WvStreamsDebuggerServer::Connection::send(WvStringParm code,
        WvStringParm result)
{
    WvStringList results;
    results.append(result);
    send(code, results);
}


void WvStreamsDebuggerServer::Connection::send(WvStringParm code,
        WvStringList &results)
{
    print("%s %s\n", wvtcl_escape(code), wvtcl_encode(results));
}


WvStreamsDebuggerServer::WvStreamsDebuggerServer(const WvUnixAddr &unix_addr,
        AuthCallback _auth_cb,
        const WvIPPortAddr &tcp_addr) :
    log("WvStreamsDebuggerServer", WvLog::Debug3),
    unix_listener(NULL),
    tcp_listener(NULL),
    auth_cb(_auth_cb)
{
    WvIStreamList::globallist.append(&streams, false);

#ifndef _WIN32
    if (true)
    {
        unix_listener = new WvUnixListener(unix_addr, 0700);
        unix_listener->set_wsname("wsd listener on %s", unix_addr);
        unix_listener->setcallback(
	    wv::bind(&WvStreamsDebuggerServer::unix_listener_cb, this));
        unix_listener->setclosecallback(
	    wv::bind(&WvStreamsDebuggerServer::unix_listener_close_cb, this));
        streams.append(unix_listener, true);
        log("Listening on %s\n", unix_addr);
    }
#endif
    
    if (tcp_addr != WvIPPortAddr())
    {
        tcp_listener = new WvTCPListener(tcp_addr);
        tcp_listener->set_wsname("wsd listener on %s", tcp_addr);
        tcp_listener->setcallback(
	    wv::bind(&WvStreamsDebuggerServer::tcp_listener_cb, this));
        tcp_listener->setclosecallback(
	    wv::bind(&WvStreamsDebuggerServer::tcp_listener_close_cb, this));
        streams.append(tcp_listener, true);
        log("Listening on %s\n", tcp_addr);
    }
}


WvStreamsDebuggerServer::~WvStreamsDebuggerServer()
{
    WvIStreamList::globallist.unlink(&streams);
}


#ifndef _WIN32
void WvStreamsDebuggerServer::unix_listener_cb()
{
    WvUnixConn *unix_conn = unix_listener->accept();
    if (!unix_conn)
        return;
    log("Accepted connection from %s\n", *unix_conn->src());
    Connection *conn = new Connection(unix_conn);
    conn->setcallback(wv::bind(&WvStreamsDebuggerServer::ready_cb, this,
			       wv::ref(*conn)));
    streams.append(conn, true);
}


void WvStreamsDebuggerServer::unix_listener_close_cb()
{
    log("Listener on %s closing\n", *unix_listener->src());
}
#endif

void WvStreamsDebuggerServer::tcp_listener_cb()
{
    WvTCPConn *tcp_conn = tcp_listener->accept();
    if (!tcp_conn)
        return;
    log("Accepted connection from %s\n", *tcp_conn->src());
    Connection *conn = new Connection(tcp_conn);
    conn->setcallback(wv::bind(&WvStreamsDebuggerServer::ready_cb, this,
			       wv::ref(*conn)));
    streams.append(conn, true);
}


void WvStreamsDebuggerServer::tcp_listener_close_cb()
{
    log("Listener on %s closing\n", *tcp_listener->src());
}


void WvStreamsDebuggerServer::auth_request_cb(Connection &s)
{
    s.choose_salt();
    s.send("AUTH", s.salt);
    
    s.setcallback(wv::bind(&WvStreamsDebuggerServer::auth_response_cb, this,
			   wv::ref(s)));
}


void WvStreamsDebuggerServer::auth_response_cb(Connection &s)
{
    const char *line = s.getline();
    if (line == NULL)
        return;
        
    WvStringList args;
    wvtcl_decode(args, line);
    
    WvString username = args.popstr();
    WvString encoded_salted_password = args.popstr();
    
    if (!auth_cb || !username || !encoded_salted_password
        || !auth_cb(username, s.salt, encoded_salted_password))
    {
        s.send("ERROR", "Authentication failure");
        s.setcallback(wv::bind(&WvStreamsDebuggerServer::auth_request_cb,
			       this, wv::ref(s)));
    }
    else
    {
        s.send("OK", "Authenticated");
        s.setcallback(wv::bind(&WvStreamsDebuggerServer::ready_cb, this,
			       wv::ref(s)));
    }
}


void WvStreamsDebuggerServer::ready_cb(Connection &s)
{
    const char *line = s.getline();
    if (line == NULL)
        return;
        
    WvStringList args;
    wvtcl_decode(args, line);
    
    WvString cmd = args.popstr();
    if (!cmd)
    {
        s.send("ERROR", "Empty command");
        return;
    }
    
    WvString result = s.debugger.run(cmd, args,
				     wv::bind(&Connection::result_cb, &s,
					      wv::_1, wv::_2));
    if (!!result)
        s.send("ERROR", result);
    else
        s.send("OK", "Command successful");
}


