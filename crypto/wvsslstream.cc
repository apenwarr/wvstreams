#include "wvsslstream.h"
#include <ssl.h>
#include <err.h>
#include <assert.h>

WvSSLStream::WvSSLStream(WvStream *_slave, WvX509Mgr *x509, bool _verify,
		         bool _is_server)
    : WvStreamClone(&slave), debug("WvSSLStream",WvLog::Debug5)
{
    slave = _slave;
    verify = _verify;
    is_server = _is_server;
    read_again = false;
    writeonly = 1400;
    
    if (is_server && (x509 == NULL))
    {
	seterr("Certificate not available: server mode not possible!");
	return;
    }

    SSL_load_error_strings();
    SSL_library_init();
    debug("SSL library initialized.\n");

    ctx = NULL;
    ssl = NULL;
    sslconnected = false;

    if (is_server)
    {
	meth = SSLv23_server_method();
    	debug("Configured algorithms and methods for server mode.\n");

	ctx = SSL_CTX_new(meth);
    	if (!ctx)
    	{
	    seterr("Can't get SSL context!");
	    return;
    	}
	// Allow SSL Writes to only write part of a request...
	SSL_CTX_set_mode(ctx,SSL_MODE_ENABLE_PARTIAL_WRITE);

	if (SSL_CTX_use_certificate(ctx, x509->cert) <= 0)
	{
	    seterr("Error loading certificate!");
	    return;
	}
    	debug("Certificate activated.\n");

	if (SSL_CTX_use_RSAPrivateKey(ctx, x509->keypair->rsa) <= 0)
	{
	    seterr("Error loading RSA private key!");
	    return;
	}
	debug("RSA private key activated.\n");
	debug("Server mode ready.\n");
    }
    else
    {
    	meth = SSLv23_client_method();
    	debug("Configured algorithms and methods for client mode.\n");
    
    	ctx = SSL_CTX_new(meth);
    	if (!ctx)
    	{
	    seterr("Can't get SSL context!");
	    return;
    	}
    }
    
    ssl = SSL_new(ctx);
    if (!ssl)
    {
    	seterr("Can't create SSL object!");
	return;
    }

    debug("SSL stream initialized.\n");

    // make sure we run the SSL_connect once, after our stream is writable
    slave->force_select(false, true);
}


WvSSLStream::~WvSSLStream()
{
    close();
    
    debug("Shutting down SSL connection.\n");
    if (geterr())
	debug("Error was: %s\n", errstr());
    
    if (slave)
	delete slave;
}

 
size_t WvSSLStream::uread(void *buf, size_t len)
{
    if (!sslconnected)
	return 0;
    
    int result = SSL_read(ssl, (char *)buf, len);
    
    if (len > 0 && result == 0) // read no bytes, though we wanted some
	close(); // EOF
    else if (result < 0)
    {
	if (errno != EAGAIN)
	    seterr(errno);
        return 0;
    }
    
    if (len)
    {
	if ((size_t)result == len)
	    read_again = true;
	else
	    read_again = false;
    }
    
    debug("<< %s bytes\n", result);
    
    return result;
}


size_t WvSSLStream::uwrite(const void *buf, size_t len)
{
    if (!sslconnected)
    {
	debug(">> writing, but not connected yet; enqueue.\n");
	return 0;
    }

    debug(">> I want to write %s bytes.\n", len);

    // copy buf into the bounce buffer...

    memcpy(bouncebuffer,buf,(writeonly < len) ? writeonly : len); 

    int result = SSL_write(ssl, bouncebuffer,
			   (writeonly < len) ? writeonly : len);

    if (result < 0)
    {
	int errcode = SSL_get_error(ssl, result);
	
        switch (errcode)
	{
	case SSL_ERROR_WANT_WRITE:
	    debug(">> SSL_write() needs to wait for writable.\n");
	    writeonly = len;
	    break;
	    
	case SSL_ERROR_NONE:
	    // We shouldn't ever get here, but handle it nicely anyway... 
	    debug(">> SSL_write non-error!\n");
	    break;
	    
	default:
	    debug(">> ERROR: SSL_write() call failed.\n");
	    seterr(WvString("SSL write error #%s", errcode));
	    break;
	}
	return 0;
    }
    else
        writeonly = 1400;

    debug(">> SSL wrote %s bytes; wanted to write %s bytes.\n",
	   result, len);

    return result;
}
 

void WvSSLStream::close()
{
    if (ssl)
    {
	SSL_shutdown(ssl);
	SSL_free(ssl);
	ssl = NULL;
    }
    
    WvStreamClone::close();
    
    if (ctx)
    {
	SSL_CTX_free(ctx);
	ctx = NULL;
    }
}


bool WvSSLStream::pre_select(SelectInfo &si)
{
    // the SSL library might be keeping its own internal buffers - try
    // reading again if we were full the last time.
    if (si.wants.readable && read_again)
    {
	debug("pre_select: try reading again immediately.\n");
	return true;
    }

    return WvStreamClone::pre_select(si);
}

 
bool WvSSLStream::post_select(SelectInfo &si)
{
    bool result = WvStreamClone::post_select(si);

    // SSL takes a few round trips to
    // initialize itself, and we mustn't block in the constructor, so keep
    // trying here... it is also turning into a rather cool place
    // to do the validation of the connection ;)
    if (!sslconnected && slave && slave->isok() && result)
    {
	slave->undo_force_select(false, true, false);
	
	// for ssl streams to work, we have to be cloning a stream that
	// actually uses a single, valid fd.
	assert(getrfd() >= 0);
	assert(getwfd() >= 0);
	assert(getwfd() == getrfd());
	SSL_set_fd(ssl, getrfd());
	debug("SSL connected on fd %s.\n", getrfd());
	
	int err;
    
	if (is_server)
	{
	    // If we are a server, get ready to accept an incoming SSL
	    // connection
	    err = SSL_accept(ssl);
	}
	else
	    err = SSL_connect(ssl);
	
	if (err < 0)
	{
	    if (errno == EAGAIN)
		debug("Still waiting for SSL negotiation.\n");
	    else if (!errno)
		seterr(WvString("SSL negotiation failed (%s)!", err));
	    else
		seterr(errno);
	}
	else  // We're connected, so let's do some checks ;)
	{
	    debug("SSL connection using cipher %s.\n", SSL_get_cipher(ssl));
	    if (verify)
	    {
	    	WvX509Mgr peercert(SSL_get_peer_certificate(ssl));
	    	if (peercert.validate() && !peercert.err)
	    	{
		    sslconnected = true;
	    	    debug("SSL finished negotiating - certificate is valid.\n");
	    	}
	    	else
	    	{
		    if (peercert.err)
			seterr(peercert.errstr);
		    else
			seterr("Peer certificate is invalid!");
	    	}
	    }
	    else
	    {
		sslconnected = true;
		debug("SSL finished negotiating "
		      "- certificate validation disabled.\n");
	    }	
	} 
	
	return false;
    }
    else
	return result;
}

