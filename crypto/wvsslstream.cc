#include "wvsslstream.h"
#include <ssl.h>
#include <err.h>
#include <assert.h>


WvSSLStream::WvSSLStream(WvStream *_slave)
    : WvStreamClone(&slave), debug("WvSSLStream",WvLog::Debug5)
{
    sslconnected = false;
    slave = _slave;
    
    ctx = NULL;
    ssl = NULL;
    
    SSL_load_error_strings();
    SSL_library_init();
    debug("SSL Library Initialization Finished\n");
    
    meth = SSLv23_client_method();
    debug("Setting Algorithms, and Methods Complete\n");
    
    ctx = SSL_CTX_new(meth);
    if (!ctx)
    {
	seterr("Can't get SSL context!");
	return;
    }

    debug("SSL Context Set up\n");
    
    ssl = SSL_new(ctx);
    if (!ssl)
    {
	seterr("Can't create SSL object!");
	return;
    }
	
    debug("SSL Connector initialized\n");
    
    // make sure we run the SSL_connect once our stream is writable
    slave->force_select(true, true, false);
}


WvSSLStream::~WvSSLStream()
{
    debug("Shutting down SSL Connection\n");
    close();
    
    SSL_free(ssl);
    SSL_CTX_free(ctx);
}

 
size_t WvSSLStream::uread(void *buf, size_t len)
{
    if (!sslconnected)
	return 0;
    
    int err;
    
    err = SSL_read(ssl, (char *) buf, len);
    if (len > 0 && err == 0)
	close();
    else if (err < 0)
    {
	if (errno != EAGAIN)
	    seterr(errno);
        return 0;
    }
    
    debug("<< %s bytes\n",err);
    
    return err;
}


size_t WvSSLStream::uwrite(const void *buf, size_t len)
{
    if (!sslconnected)
	return 0;
    
    int err;
    err = SSL_write(ssl, (char *) buf, len);
    if (len > 0 && err == 0)
	close();
    else if (err < 0)
    {
	if (errno != EAGAIN)
	    seterr(errno);
        return 0;
    }
    
    debug(">> %s bytes\n",err);
    return err;
}
 

void WvSSLStream::close()
{
    SSL_shutdown(ssl);
    WvStreamClone::close();
}

 
void WvSSLStream::execute()
{
    int err;
    
    WvStreamClone::execute();
    
    // This little nasty kludge is here because SSL takes a second to
    // initialize itself, and we can't block in the constructor, so keep
    // trying here...
    if (!sslconnected && slave && slave->select(0, false, true))
    {
	slave->force_select(false, false, false);
	
	// for ssl streams to work, we have to be cloning a stream that
	// actually uses a valid fd.
	assert(getfd() >= 0);
	SSL_set_fd(ssl, getfd());
	debug("SSL Connected to WvStream %s\n",getfd());
    
	err = SSL_connect(ssl);
	if (err < 0)
	{
	    if (errno == EAGAIN)
		debug("Still Waiting for SSL Connection\n");
	    else
		seterr("SSL negotiation failed!");
	}
	else
	{
	    sslconnected = true;
	    debug("SSL Connection now running\n");
	} 
    }
}

