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


WvStreamsDebuggerServer::WvStreamsDebuggerServer(WvUnixAddr unix_addr,
        AuthCallback _auth_cb,
        WvIPPortAddr tcp_addr) :
    log("WvStreamsDebuggerServer", WvLog::Debug3),
    unix_listener(NULL),
    tcp_listener(NULL),
    auth_cb(_auth_cb)
{
    if (true)
    {
        unix_listener = new WvUnixListener(unix_addr, 0700);
        unix_listener->set_wsname("wsd listener on %s", unix_addr);
        unix_listener->setcallback(WvStreamCallback(this,
                &WvStreamsDebuggerServer::unix_listener_cb), NULL);
        unix_listener->setclosecallback(IWvStreamCallback(this,
                &WvStreamsDebuggerServer::unix_listener_close_cb));
        WvIStreamList::globallist.append(unix_listener, true);
        streams.append(unix_listener, false);
        log("Listening on %s\n", unix_addr);
    }
    
    if (tcp_addr != WvIPPortAddr())
    {
        tcp_listener = new WvTCPListener(tcp_addr);
        tcp_listener->set_wsname("wsd listener on %s", tcp_addr);
        tcp_listener->setcallback(WvStreamCallback(this,
                &WvStreamsDebuggerServer::tcp_listener_cb), NULL);
        tcp_listener->setclosecallback(IWvStreamCallback(this,
                &WvStreamsDebuggerServer::tcp_listener_close_cb));
        WvIStreamList::globallist.append(tcp_listener, true);
        streams.append(tcp_listener, false);
        log("Listening on %s\n", tcp_addr);
    }
}


WvStreamsDebuggerServer::~WvStreamsDebuggerServer()
{
    WvIStreamList::Iter i(streams);
    for (i.rewind(); i.next(); )
        i->close();
}


void WvStreamsDebuggerServer::unix_listener_cb(WvStream &, void *)
{
    WvUnixConn *unix_conn = unix_listener->accept();
    if (!unix_conn)
        return;
    log("Accepted connection from %s\n", *unix_conn->src());
    Connection *conn = new Connection(unix_conn);
    conn->setcallback(WvStreamCallback(this,
            &WvStreamsDebuggerServer::ready_cb), NULL);
    WvIStreamList::globallist.append(conn, true);
    streams.append(conn, false);
}


void WvStreamsDebuggerServer::unix_listener_close_cb(WvStream &)
{
    log("Listener on %s closing\n", *unix_listener->src());
}


void WvStreamsDebuggerServer::tcp_listener_cb(WvStream &, void *)
{
    WvTCPConn *tcp_conn = tcp_listener->accept();
    if (!tcp_conn)
        return;
    log("Accepted connection from %s\n", *tcp_conn->src());
    Connection *conn = new Connection(tcp_conn);
    conn->setcallback(WvStreamCallback(this,
            &WvStreamsDebuggerServer::ready_cb), NULL);
    WvIStreamList::globallist.append(conn, true);
    streams.append(conn, false);
}


void WvStreamsDebuggerServer::tcp_listener_close_cb(WvStream &)
{
    log("Listener on %s closing\n", *tcp_listener->src());
}


void WvStreamsDebuggerServer::auth_request_cb(WvStream &_s, void *)
{
    Connection &s = static_cast<Connection &>(_s);
    
    s.choose_salt();
    s.send("AUTH", s.salt);
    
    s.setcallback(WvStreamCallback(this,
            &WvStreamsDebuggerServer::auth_response_cb), NULL);
}


void WvStreamsDebuggerServer::auth_response_cb(WvStream &_s, void *)
{
    Connection &s = static_cast<Connection &>(_s);
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
        s.setcallback(WvStreamCallback(this,
                &WvStreamsDebuggerServer::auth_request_cb), NULL);
    }
    else
    {
        s.send("OK", "Authenticated");
        s.setcallback(WvStreamCallback(this,
                &WvStreamsDebuggerServer::ready_cb), NULL);
    }
}


void WvStreamsDebuggerServer::ready_cb(WvStream &_s, void *)
{
    Connection &s = static_cast<Connection &>(_s);
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
        WvStreamsDebugger::ResultCallback(&s, &Connection::result_cb));
    if (!!result)
        s.send("ERROR", result);
    else
        s.send("OK", "Command successful");
}


void WvStreamsDebuggerServer::close_cb(WvStream &_s)
{
    streams.unlink(&_s);
}
