/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * SSL (Socket Security Layer) communications via WvStreams.
 */ 
#ifndef __WVSSLSTREAM_H
#define __WVSSLSTREAM_H

#include "wvstreamclone.h"
#include "wvfdstream.h"
#include "wvlog.h"
 
struct ssl_st;
struct ssl_ctx_st;
struct ssl_method_st;

typedef struct ssl_ctx_st SSL_CTX;
typedef struct ssl_st SSL;
typedef struct ssl_method_st SSL_METHOD;

class WvX509Mgr;

/**
 * SSL Stream, handles SSLv2, SSLv3, and TLS
 * Methods - If you want it to be a server, then you must feed the constructor
 * a WvX509Mgr object
 */
class WvSSLStream : public WvStreamClone
{
public:
    /**  
     * Start an SSL connection on the stream _slave.  The x509 structure
     * is optional for a client, and mandatory for a server.
     */
    WvSSLStream(IWvStream *_slave, WvX509Mgr *x509 = NULL, 
    		bool _verify = false, bool _is_server = false);
    
    /**
     * Cleans up everything (calls close + frees up the SSL Objects used)
     */
    virtual ~WvSSLStream();
    
    virtual bool pre_select(SelectInfo &si);
    virtual bool post_select(SelectInfo &si);
    
    virtual void close();
    
    virtual bool isok() const;
    
protected:
    /**
     * SSL Context - used to create SSL Object
     */
    SSL_CTX *ctx;
    
    /**
     * Main SSL Object - after SSL_set_fd() we make all calls through the connection
     * through here
     */
    SSL *ssl;
    
    /**
     * Again, used to setup the SSL Object - The Method is set so that this client can
     * Connect to, and understand SSLv2, SSLv3, and TLS servers
     */
    SSL_METHOD *meth;
    
    /**
     * Overrides the standard write function, and use
     * SSL_write() instead...
     */
    virtual size_t uwrite(const void *buf, size_t len);
    
    /**
     * Overrides for the standard read function, so that SSL_read() will
     * get called...
     */
    virtual size_t uread(void *buf, size_t len);
    
private:
    /**
     * Connection Status Flag, since SSL takes a few seconds to
     * initialize itself.
     */
    volatile bool sslconnected;
    
    /**
     * Keep track of whether we are a client or a server
     */
    bool is_server;
    
    /**
     * Keep track of whether we want to check the peer who connects to us
     */
    bool verify;
    
    /**
     * Internal Log Object
     */
    WvLog debug;

    /**
     * SSL_write() may return an SSL_ERROR_WANT_WRITE code which
     * indicates that the function should be called again with
     * precisely the same arguments as the last time.  To ensure that
     * this can happen, we must unfortunately copy data into a bounce
     * buffer and remeber the fact.  We use a WvBuffer here to allow
     * an arbitrary amount of data to be set aside.
     */
    WvInPlaceBuffer write_bouncebuf;
    size_t write_eat;

    /**
     * Similar nastiness happens with SSL_read()
     */
    WvInPlaceBuffer read_bouncebuf;
    bool read_pending;
};

#endif // __WVSSLSTREAM_H

