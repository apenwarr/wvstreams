/*
 * Insert Appropriate Copyright header here....
 * I really don't care just mention me in a changelog somewhere ;) ppatters.
 */
#include "wvstreamclone.h"
#include "wvlog.h"
 
struct ssl_st;
struct ssl_ctx_st;
struct ssl_method_st;

typedef struct ssl_ctx_st SSL_CTX;
typedef struct ssl_st SSL;
typedef struct ssl_method_st SSL_METHOD;

/**
 * SSL Client Side (for now) Stream, handles SSLv2, SSLv3, and TLS
 * Methods
 */
class WvSSLStream : public WvStreamClone
{
public:
    /**  
     * Start an SSL Connection on the stream _slave 
     */
    WvSSLStream(WvStream *_slave);
    
    /**
     * Cleans up everything (calls close + frees up the SSL Objects used)
     */
    virtual ~WvSSLStream();
    
    /**
     * Need an execute function for this stream because
     * the initial connection takes a bit of fiddling to set
     * up.
     */   
    virtual void execute();
    
    /**
     * Close down the SSL Connection
     */
    virtual void close();
    
protected:
    WvStream *slave;
    
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
     * Overrider the standard write function, and use
     * SSL_write() instead...
     */
    size_t     uwrite(const void *buf, size_t len);
    
    /**
     * Overrides for the standard read function, so that SSL_read() will
     * get called...
     */
    size_t     uread(void *buf, size_t len);
    
private:
    /// Connection Status Flag, since SSL takes a few seconds to
    /// initialize itself.
    bool       sslconnected;
    
    /**
     * Internal Log Object
     */
    WvLog      debug;
};

