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
	seterr("Certificate not available, so server mode not possible");
	return;
    }

    SSL_load_error_strings();
    SSL_library_init();
    debug("SSL Library Initialization Finished\n");

    ctx = NULL;
    ssl = NULL;
    sslconnected = false;

    if (is_server)
    {
	meth = SSLv23_server_method();
    	debug("Setting Algorithms, and Methods Complete\n");

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
	    seterr("Error loading Certificate!");
	    return;
	}
    	debug("Setting Certificate Complete\n");

	if (SSL_CTX_use_RSAPrivateKey(ctx, x509->keypair->rsa) <= 0)
	{
	    seterr("Error loading RSA Private Key!");
	    return;
	}
	debug("Setting Private Key Complete\n");
	debug("Ready to go for Server mode\n");
    }
    else
    {
    	meth = SSLv23_client_method();
    	debug("Setting Algorithms, and Methods Complete\n");
    
    	ctx = SSL_CTX_new(meth);
    	if (!ctx)
    	{
	    seterr("Can't get SSL context!");
	    return;
    	}

    	debug("SSL Context Set up\n");
    
    }
    ssl = SSL_new(ctx);
    if (!ssl)
    {
    	seterr("Can't create SSL object!");
	return;
    }

    debug("SSL Connector initialized\n");

    // make sure we run the SSL_connect once, after our stream is writable
    slave->force_select(true, true, false);
}


WvSSLStream::~WvSSLStream()
{
    debug("Shutting down SSL Connection\n");
    close();
    
    if (slave)
	delete slave;
}

 
size_t WvSSLStream::uread(void *buf, size_t len)
{
    if (!sslconnected)
	return 0;
    
    int result = SSL_read(ssl, (char *)buf, len);
    
    if (len > 0 && result == 0)
	close();
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
    debug(">> I want to write %s bytes\n",len);

    if (!sslconnected)
    {
	debug(">> EEEEP! I can't find my sslconnection!\n");
	return 0;
    }

    // copy buf into the bounce buffer...

    memcpy(bouncebuffer,buf,(writeonly < len) ? writeonly : len); 

    int result = SSL_write(ssl, bouncebuffer, (writeonly < len) ? writeonly : len);

    if (len > 0 && result == 0)
	close();
    else if (result < 0)
    {
        switch(SSL_get_error(ssl,result))
	{
	   case SSL_ERROR_WANT_WRITE:
		debug(">> ERROR: SSL_write() cannot complete at this time...retry!\n");
		writeonly = len;
		break;
	   case SSL_ERROR_NONE:
		debug(">> Hmmm... something got confused... no SSL Errors!\n");
		break;
	   default:
		debug(">> ERROR: SSL_write() call failed\n");
		seterr("SSL Write failed - bailing out of the SSL Session");
		break;
	}
	return 0;
    }
    else
    {
        writeonly = 1400;
    }

    debug(">> SSL wrote %s bytes \t WvStreams wanted to write %s bytes\n", 
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


bool WvSSLStream::select_setup(SelectInfo &si)
{
    // the SSL library might be keeping its own internal buffers - try
    // reading again if we were full the last time.
    if (si.readable && read_again)
    {
	debug("Have to try reading again!\n");
	return true;
    }

    return WvStreamClone::select_setup(si);

}

 
bool WvSSLStream::test_set(SelectInfo &si)
{
    bool result = WvStreamClone::test_set(si);

    // SSL takes a few round trips to
    // initialize itself, and we mustn't block in the constructor, so keep
    // trying here... it is also turning into a rather cool place
    // to do the validation of the connection ;)
    if (!sslconnected && slave && slave->isok() && result)
    {
	slave->force_select(false, false, false);
	
	// for ssl streams to work, we have to be cloning a stream that
	// actually uses a valid fd.
	assert(getfd() >= 0);
	SSL_set_fd(ssl, getfd());
	debug("SSL Connected to WvStream %s\n",getfd());
	
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
		debug("Still Waiting for SSL Connection\n");
	    else
		seterr("SSL negotiation failed!");
	}
	else  // We're connected, so let's do some checks ;)
	{
	    debug("SSL Connection using %s\n", SSL_get_cipher(ssl));
	    if (verify)
	    {
	    	WvX509Mgr peercert(SSL_get_peer_certificate(ssl));
	    	if (peercert.validate() && !peercert.err)
	    	{
		    sslconnected = true;
	    	    debug("SSL Connection now running\n");
	    	}
	    	else
	    	{
		    if (peercert.err)
			seterr(peercert.errstr);
		    else
			seterr("Peer Certificate cannot be validated");
	    	}
	    }
	    else
	    {
		sslconnected = true;
		debug("SSL Connection now running\n");
	    }	
	} 
	
	return false;
    }
    else
	return result;
}

