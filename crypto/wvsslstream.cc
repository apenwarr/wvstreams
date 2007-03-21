/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */
#define OPENSSL_NO_KRB5
#include "wvsslstream.h"
#include "wvx509.h"
#include "wvcrypto.h"
#include "wvmoniker.h"
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <assert.h>

#ifndef _WIN32
# if HAVE_ARGZ_H
#  include <argz.h>
# else
#  if HAVE_ERRNO_H
#   include <errno.h>
#  endif
# endif
#else
#undef errno
#define errno GetLastError()
// FIXME: WLACH: seems to cause an error on mingw32 3.4.2.. is this needed?
//typedef DWORD error_t;
#undef EAGAIN
#define EAGAIN WSAEWOULDBLOCK
#endif

static IWvStream *creator(WvStringParm s)
{
    return new WvSSLStream(wvcreate<IWvStream>(s), NULL, 0, false);
}

static IWvStream *screator(WvStringParm s)
{
    return new WvSSLStream(wvcreate<IWvStream>(s), NULL, 0, true);
}

static WvMoniker<IWvStream> reg("ssl", creator);
static WvMoniker<IWvStream> sreg("sslserv", screator);


#define MAX_BOUNCE_AMOUNT (16384) // 1 SSLv3/TLSv1 record

static int wv_verify_cb(int preverify_ok, X509_STORE_CTX *ctx) 
{
   // This is just returns true, since what we really want
   // is for the WvSSLValidateCallback to do this work
   return 1;
}

WvSSLStream::WvSSLStream(IWvStream *_slave, WvX509Mgr *_x509,
    WvSSLValidateCallback _vcb, bool _is_server) :
    WvStreamClone(_slave), debug("WvSSLStream", WvLog::Debug5),
    write_bouncebuf(MAX_BOUNCE_AMOUNT), write_eat(0),
    read_bouncebuf(MAX_BOUNCE_AMOUNT), read_pending(false)
{
    x509 = _x509;
    if (x509)
	x509->addRef(); // openssl may keep a pointer to this object
    
    vcb = _vcb;
    is_server = _is_server;
    ctx = NULL;
    ssl = NULL;
    //meth = NULL;
    sslconnected = ssl_stop_read = ssl_stop_write = false;
    
    wvssl_init();
    
    if (x509 && !x509->isok())
    {
	seterr("Cert: %s", x509->errstr());
	return;
    }

    if (is_server && !x509)
    {
	seterr("Certificate not available: server mode not possible!");
	return;
    }

    if (is_server)
    {
    	debug("Configured algorithms and methods for server mode.\n");

	ctx = SSL_CTX_new(SSLv23_server_method());
    	if (!ctx)
    	{
            ERR_print_errors_fp(stderr);
            debug("Can't get SSL context! Error: %s\n", 
                  ERR_reason_error_string(ERR_get_error()));
	    seterr("Can't get SSL context!");
	    return;
    	}
	
	// Allow SSL Writes to only write part of a request...
	SSL_CTX_set_mode(ctx, SSL_MODE_ENABLE_PARTIAL_WRITE);

	// Tell SSL to use 128 bit or better ciphers - this appears to
	// be necessary for some reason... *sigh*
	SSL_CTX_set_cipher_list(ctx, "HIGH");

	// Enable the workarounds for broken clients and servers
	// and disable the insecure SSLv2 protocol
        SSL_CTX_set_options(ctx, SSL_OP_ALL|SSL_OP_NO_SSLv2);

	if (!x509->bind_ssl(ctx))
	{
	    seterr("Unable to bind Certificate to SSL Context!");
	    return;
	}
	
	if (!!vcb)
            SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER|SSL_VERIFY_CLIENT_ONCE, 
                               wv_verify_cb);
	
	debug("Server mode ready.\n");
    }
    else
    {
    	debug("Configured algorithms and methods for client mode.\n");
    
    	ctx = SSL_CTX_new(SSLv23_client_method());
    	if (!ctx)
    	{
	    seterr("Can't get SSL context!");
	    return;
    	}
        if (x509 && !x509->bind_ssl(ctx))
        {
            seterr("Unable to bind Certificate to SSL Context!");
            return;
        }
    }
    
    //SSL_CTX_set_read_ahead(ctx, 1);

    ERR_clear_error();
    ssl = SSL_new(ctx);
    if (!ssl)
    {
    	seterr("Can't create SSL object!");
	return;
    }

    if (!!vcb)
	SSL_set_verify(ssl, SSL_VERIFY_PEER|SSL_VERIFY_CLIENT_ONCE, 
                       wv_verify_cb);



    debug("SSL stream initialized.\n");

    // make sure we run the SSL_connect once, after our stream is writable
    force_select(false, true);
}


WvSSLStream::~WvSSLStream()
{
    close();
    
    debug("Deleting SSL connection.\n");
    if (geterr())
	debug("Error was: %s\n", errstr());
    
    WVRELEASE(x509);
    wvssl_free();
}


void WvSSLStream::printerr(WvStringParm func)
{
    unsigned long l = ERR_get_error();
    char buf[121];      // man ERR_error_string says must be > 120.

    SSL_load_error_strings();
    while (l)
    {
        ERR_error_string(l, buf);
        debug("%s error: %s\n", func, buf);
        l = ERR_get_error();
    }
    ERR_free_strings();
}

 
size_t WvSSLStream::uread(void *buf, size_t len)
{
    if (!sslconnected)
        return 0;
    if (len == 0) return 0;

    // if SSL buffers stuff on its own, select() may not wake us up
    // the next time around unless we're sure there is nothing left
    read_pending = true;
    
    size_t total = 0;
    for (;;)
    {
        // handle SSL_read quirk
        if (read_bouncebuf.used() != 0)
        {
            // copy out cached data
            size_t amount = len < read_bouncebuf.used() ?
                len : read_bouncebuf.used();
            read_bouncebuf.move(buf, amount);

            // locate next chunk in buffer
            len -= amount;
            total += amount;
            if (len == 0)
	    {
		read_pending = false;
                break;
	    }
            buf = (unsigned char *)buf + amount;
	    
	    // FIXME: this shouldn't be necessary, but it resolves weird
	    // problems when the other end disconnects in the middle of
	    // SSL negotiation, but only on emakela's machine.  I don't
	    // know why.  -- apenwarr (2004/02/10)
	    break;
        }

        // attempt to read
        read_bouncebuf.zap(); // force use of same position in buffer
        size_t avail = read_bouncebuf.free();
        unsigned char *data = read_bouncebuf.alloc(avail);
        
	ERR_clear_error();
        int result = SSL_read(ssl, data, avail);
	// debug("<< SSL_read result %s for %s bytes (wanted %s)\n",
	//      result, avail, len);
        if (result <= 0)
        {
	    error_t err = errno;
            read_bouncebuf.unalloc(avail);
            int sslerrcode = SSL_get_error(ssl, result);
	    switch (sslerrcode)
            {
                case SSL_ERROR_WANT_READ:
		    debug("<< SSL_read() needs to wait for writable.\n");
                    break; // wait for later
                case SSL_ERROR_WANT_WRITE:
		    debug("<< SSL_read() needs to wait for readable.\n");
                    break; // wait for later
                    
                case SSL_ERROR_NONE:
                    break; // no error, but can't make progress
                    
	        case SSL_ERROR_ZERO_RETURN:
		    debug("<< EOF: zero return\n");
		
		    // don't do this if we're returning nonzero!
		    // (SSL has no way to do a one-way shutdown, so if SSL
		    // detects a read problem, it's also a write problem.)
		    if (!total) { noread(); nowrite(); }
                    break;

		case SSL_ERROR_SYSCALL:
		    if (!err)
		    {
			if (result == 0)
			{
			    debug("<< EOF: syscall error "
				  "(%s/%s, %s/%s) total=%s\n",
				  stop_read, stop_write,
				  isok(), cloned && cloned->isok(), total);
			    
			    // don't do this if we're returning nonzero!
			    // (SSL has no way to do a one-way shutdown, so
			    // if SSL detects a read problem, it's also a
			    // write problem.)
			    if (!total) { noread(); nowrite(); }
			}
		    }
		    else
		    {
			debug("<< SSL_read() err=%s (%s)\n",
			    err, strerror(err));
			seterr_both(err, WvString("SSL read: %s",
			    strerror(err)));
		    }
		    break;
                    
                default:
                    printerr("SSL_read");
                    seterr("SSL read error #%s", sslerrcode);
                    break;
            }
            read_pending = false;
            break; // wait for next iteration
        }
	// debug("<< read result was %s\n", result);
	
	if (result < 0)
	    result = 0;
        read_bouncebuf.unalloc(avail - result);
    }

    // debug("<< read %s bytes (%s, %s)\n",
    //	  total, isok(), cloned && cloned->isok());
    return total;
}


size_t WvSSLStream::uwrite(const void *buf, size_t len)
{
    if (!sslconnected)
    {
	debug(">> writing, but not connected yet (%s); enqueue.\n", getwfd());
        unconnected_buf.put(buf, len);
	return len;
    }

    if (len == 0) return 0;

//    debug(">> I want to write %s bytes.\n", len);

    size_t total = 0;
    
    // eat any data that was precached and already written
    if (write_eat >= len)
    {
        write_eat -= len;
        total = len;
        len = 0;
    }
    else
    {
        buf = (const unsigned char *)buf + write_eat;
        total = write_eat;
        len -= write_eat;
        write_eat = 0;
    }

    // FIXME: WOW!!! Ummm... hope this never spins...
    // 
    for (;;) 
    {
        // handle SSL_write quirk
        if (write_bouncebuf.used() == 0)
        {
            if (len == 0) break;

            // copy new data into the bounce buffer only if empty
            // if it were not empty, then SSL_write probably returned
            // SSL_ERROR_WANT_WRITE on the previous call and we
            // must invoke it with precisely the same arguments
            size_t amount = len < write_bouncebuf.free() ?
                len : write_bouncebuf.free();
            write_bouncebuf.put(buf, amount);
            // note: we don't adjust the total yet...
        } // otherwise we use what we cached last time in bounce buffer
        
        // attempt to write
        size_t used = write_bouncebuf.used();
        const unsigned char *data = write_bouncebuf.get(used);

        ERR_clear_error();
        int result = SSL_write(ssl, data, used);
	// debug("<< SSL_write result %s for %s bytes\n",
	//      result, used);
        if (result <= 0)
        {
            int sslerrcode = SSL_get_error(ssl, result);
            write_bouncebuf.unget(used);
            switch (sslerrcode)
            {
                case SSL_ERROR_WANT_READ:
                    debug(">> SSL_write() needs to wait for readable.\n");
                    break; // wait for later
                case SSL_ERROR_WANT_WRITE:
                    // debug(">> SSL_write() needs to wait for writable.\n");
                    break; // wait for later
                    
	        case SSL_ERROR_SYSCALL:
		    debug(">> ERROR: SSL_write() failed on socket error.\n");
		    seterr(WvString("SSL write error: %s", strerror(errno)));
		    break;
	    
	        // This case can cause truncated web pages... give more info
	        case SSL_ERROR_SSL:
		    debug(">> ERROR: SSL_write() failed on internal error.\n");
		    seterr(WvString("SSL write error: %s", 
				    ERR_error_string(ERR_get_error(), NULL)));
		    break;
		
	        case SSL_ERROR_NONE:
                    break; // no error, but can't make progress
                    
                case SSL_ERROR_ZERO_RETURN:
		    debug(">> SSL_write zero return: EOF\n");
                    close(); // EOF
                    break;
                    
                default:
                    printerr("SSL_write");
                    seterr(WvString("SSL write error #%s", sslerrcode));
                    break;
            }
            break; // wait for next iteration
        }
	else
	    assert((size_t)result == used);
        write_bouncebuf.zap(); // force use of same position in buffer
        
        // locate next chunk to be written
        // note: we assume that initial contents of buf and of the
        //       bouncebuf match since if we got SSL_ERROR_WANT_WRITE
        //       we did not claim to actually have written the chunk
        //       that we cached so we will have gotten it again here
        if (size_t(result) >= len)
        {
            // if we cached more previously than we were given, claim
            // we wrote what we got and remember to eat the rest later
            write_eat = result - len;
            total += len;
            break;
        }
        total += size_t(result);
        len -= size_t(result);
        buf = (const unsigned char *)buf + size_t(result);
    }
    
    //debug(">> wrote %s bytes\n", total);
    return total;
}

void WvSSLStream::close()
{
    debug("Closing SSL connection (ok=%s,sr=%s,sw=%s,child=%s).\n",
	  isok(), stop_read, stop_write, cloned && cloned->isok());
    
    if (ssl)
    {
        ERR_clear_error();
	SSL_shutdown(ssl);
	SSL_free(ssl);
	ssl = NULL;
	sslconnected = false;
    }
    
    WvStreamClone::close();
    
    if (ctx)
    {
	SSL_CTX_free(ctx);
	ctx = NULL;
    }
}


bool WvSSLStream::isok() const
{
    return ssl && WvStreamClone::isok();
}


void WvSSLStream::noread()
{
    // WARNING: openssl always needs two-way socket communications even for
    // one-way encrypted communications, so we don't pass noread/nowrite
    // along to the child stream.  This should be mostly okay, though,
    // because we'll still send it close() once we have both noread() and
    // nowrite().
    ssl_stop_read = true;
    if (ssl_stop_write)
    {
	WvStreamClone::nowrite();
	WvStreamClone::noread();
    }
}


void WvSSLStream::nowrite()
{
    // WARNING: see note in noread()
    ssl_stop_write = true;
    if (ssl_stop_read)
    {
	WvStreamClone::noread();
	WvStreamClone::nowrite();
    }
}


bool WvSSLStream::pre_select(SelectInfo &si)
{
    bool result = WvStreamClone::pre_select(si);
    // the SSL library might be keeping its own internal buffers
    // or we might have left buffered data behind deliberately
    if (si.wants.readable && (read_pending || read_bouncebuf.used()))
    {
//	debug("pre_select: try reading again immediately.\n");
	si.msec_timeout = 0;
	return true;
    }

    // if we're not ssl_connected yet, I can guarantee we're not actually
    // writable, so don't ask WvStreamClone to wake up just because *he's*
    // writable.
    bool oldwr = si.wants.writable;
    if (!sslconnected)
	si.wants.writable = !!writecb;
    result = WvStreamClone::pre_select(si);
    si.wants.writable = oldwr;

//    debug("in pre_select (%s)\n", result);
    return result;
}

 
bool WvSSLStream::post_select(SelectInfo &si)
{
    bool result = WvStreamClone::post_select(si);

    // SSL takes a few round trips to
    // initialize itself, and we mustn't block in the constructor, so keep
    // trying here... it is also turning into a rather cool place
    // to do the validation of the connection ;)
    if (!sslconnected && cloned && cloned->isok() && result)
    {
	debug("!sslconnected in post_select (r=%s/%s, w=%s/%s, t=%s)\n",
	    cloned->isreadable(), si.wants.readable,
	    cloned->iswritable(), si.wants.writable,
	    si.msec_timeout);
	
	undo_force_select(false, true, false);
	
	// for ssl streams to work, we have to be cloning a stream that
	// actually uses a single, valid fd.
        WvFDStream *fdstream = static_cast<WvFDStream*>(cloned);
        int fd = fdstream->getfd();
        assert(fd >= 0);
        ERR_clear_error();
	SSL_set_fd(ssl, fd);
//	debug("SSL connected on fd %s.\n", fd);
	
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
            {
                printerr(is_server ? "SSL_accept" : "SSL_connect");
		seterr(WvString("SSL negotiation failed (%s)!", err));
            }
	    else
            {
                printerr(is_server ? "SSL_accept" : "SSL_connect");
		seterr(errno);
            }
	}
	else  // We're connected, so let's do some checks ;)
	{
	    debug("SSL connection using cipher %s.\n", SSL_get_cipher(ssl));
	    if (!!vcb)
	    {
	    	WvX509Mgr *peercert = new WvX509Mgr(SSL_get_peer_certificate(ssl));
		debug("SSL Peer is: %s\n", peercert->get_subject());
	    	if (peercert->isok() && peercert->validate() && vcb(peercert))
	    	{
                    setconnected(true);
	    	    debug("SSL finished negotiating - certificate is valid.\n");
	    	}
	    	else
	    	{
		    if (!peercert->isok())
			seterr("Peer cert: %s", peercert->errstr());
		    else
			seterr("Peer certificate is invalid!");
	    	}
		WVRELEASE(peercert);
	    }
	    else
	    {
                setconnected(true);
		debug("SSL finished negotiating "
		      "- certificate validation disabled.\n");
	    }	
	} 
	
	return false;
    }

    if ((si.wants.readable || readcb)
	&& (read_pending || read_bouncebuf.used()))
	result = true;

    return result;
}


void WvSSLStream::setconnected(bool conn)
{
    sslconnected = conn;
    if (conn) write(unconnected_buf);
}
    
