/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * X.509 certificate management classes.
 */ 
#include "wvx509.h"
#include "wvsslhacks.h"
#include "wvdiriter.h"
#include "wvcrypto.h"
#include "wvstringlist.h"
#include "strutils.h"

#include <pem.h>
#include <x509v3.h>
#include <err.h>

static int ssl_init_count = 0;

void wvssl_init()
{
    if (!ssl_init_count)
    {
	OpenSSL_add_all_algorithms();
	ERR_load_crypto_strings();
    }
    
    ssl_init_count++;
}


void wvssl_free()
{
    ssl_init_count--;
    
    if (!ssl_init_count)
    {
	ERR_free_strings();
	EVP_cleanup();
    }
}


WvString wvssl_errstr()
{
    char buf[256];
    ERR_error_string_n(ERR_get_error(), buf, sizeof(buf));
    buf[sizeof(buf)-1] = 0;
    return buf;
}


WvX509Mgr::WvX509Mgr(X509 *_cert)
    : debug("X509",WvLog::Debug5)
{
    cert = _cert;
    rsa = NULL;
    wvssl_init();
}


WvX509Mgr::WvX509Mgr(WvString dName, int bits, WvRSAKey *_rsa)
    : debug("X509",WvLog::Debug5)
{
    debug("Creating New Certificate for %s\n",dName);
    rsa = _rsa;
    cert = NULL;
    wvssl_init();
    createSScert(dName, bits);
}


WvX509Mgr::~WvX509Mgr()
{
    if (rsa)
	delete rsa;
    X509_free(cert);
    wvssl_free();
}


// The people who designed this garbage should be shot!
// Support old versions of openssl...
#ifndef NID_domainComponent
#define NID_domainComponent 391
#endif

#ifndef NID_Domain
#define NID_Domain 392
#endif


// returns some approximation of the server's fqdn, or an empty string.
static WvString set_name_entry(X509_NAME *name, WvString dn)
{
    WvString fqdn(""), force_fqdn("");
    X509_NAME_ENTRY *ne = NULL;
    int count = 0, nid;
    
    WvStringList l;
    l.split(dn, ",");
    
    // dn is of the form: c=ca,o=foo organization,dc=foo,dc=com
    // (ie. name=value pairs separated by commas)
    WvStringList::Iter i(l);
    for (i.rewind(); i.next(); )
    {
	WvString s(*i), sid;
	char *cptr, *value;
	
	cptr = s.edit();
	value = strchr(cptr, '=');
	if (value)
	    *value++ = 0;
	else
	    value = "NULL";
	
	sid = strlwr(trim_string(cptr));
	
	if (sid == "c")
	    nid = NID_countryName;
	else if (sid == "st")
	    nid = NID_stateOrProvinceName;
	else if (sid == "o")
	    nid = NID_organizationName;
	else if (sid == "ou")
	    nid = NID_organizationalUnitName;
	else if (sid == "cn")
	{
	    nid = NID_commonName;
	    force_fqdn = value;
	}
	else if (sid == "dc")
	{
	    nid = NID_domainComponent;
	    if (!!fqdn)
		fqdn.append(".");
	    fqdn.append(value);
	}
	else if (sid == "domain")
	{
	    nid = NID_Domain;
	    force_fqdn = value;
	}
	else
	    nid = NID_domainComponent;
	
	if (!ne)
	    ne = X509_NAME_ENTRY_create_by_NID(NULL, nid,
			       V_ASN1_APP_CHOOSE, (unsigned char *)value, -1);
	else
	    X509_NAME_ENTRY_create_by_NID(&ne, nid,
			       V_ASN1_APP_CHOOSE, (unsigned char *)value, -1);
	if (!ne)
	    continue;
	X509_NAME_add_entry(name, ne, count++, 0);
    }
    
    X509_NAME_ENTRY_free(ne);
    
    if (!!force_fqdn)
	return force_fqdn;
    else
	return fqdn;
}


void WvX509Mgr::createSScert(WvString dn, int keysize)
{
    EVP_PKEY *pk;
    X509_NAME *name = NULL;
    X509_EXTENSION *ex = NULL;

    // RFC2459 says that this number must be unique for each certificate
    // issued by a CA - since this is a self-signed cert, we'll take a
    // shortcut, and give a fixed value... saves a couple of cycles rather
    // than get a random number.
    int	serial = 12345;

    WvString serverfqdn;

    if (rsa == NULL)
    {
	debug("Need a new RSA key, so generating it...\n");
	rsa = new WvRSAKey(keysize);
	debug("Ok, I've got a new RSA keypair\n");
    }

    if ((pk=EVP_PKEY_new()) == NULL)
    {
	seterr("Error creating key handler for new certificate");
	return;
    }
    if ((cert=X509_new()) == NULL)
    {
	seterr("Error creating new X509 object");
	return;
    }
    if (!EVP_PKEY_assign_RSA(pk, rsa->rsa))
    {
	seterr("Error adding RSA keys to certificate");
	return;
    }

    // Completely broken in my mind - this sets the version
    // string to '3'  (I guess version starts at 0)
    X509_set_version(cert, 0x2);

    // Set the Serial Number for the certificate
    ASN1_INTEGER_set(X509_get_serialNumber(cert), serial);

    // Set the NotBefore time to now.
    X509_gmtime_adj(X509_get_notBefore(cert), 0);

    // Now + 10 years... should be shorter, but since we don't currently
    // Have a set of routines to refresh the certificates, make it
    // REALLY long.
    X509_gmtime_adj(X509_get_notAfter(cert), (long)60*60*24*3650);
    X509_set_pubkey(cert, pk);

    name = X509_get_subject_name(cert);
    serverfqdn = set_name_entry(name, dn);
    
    if (!serverfqdn)
	serverfqdn = "null.noname.null";
				       
    X509_set_issuer_name(cert, name);
    X509_set_subject_name(cert, name);

    // Add in the netscape-specific server extension
    ex = X509V3_EXT_conf_nid(NULL, NULL, NID_netscape_cert_type, "server");
    X509_add_ext(cert, ex, -1);
    X509_EXTENSION_free(ex);

    debug("Setting netscape SSL server name extension to %s\n", serverfqdn);

    // Set the netscape server name extension to our server name
    ex = X509V3_EXT_conf_nid(NULL, NULL, NID_netscape_ssl_server_name,
			     serverfqdn.edit());
    X509_add_ext(cert, ex, -1);
    X509_EXTENSION_free(ex);

    // Set the RFC2459-mandated keyUsage field to critical, and restrict
    // the usage of this cert to digital signature and key encipherment.
    ex = X509V3_EXT_conf_nid(NULL, NULL, NID_key_usage,
			     "critical,digitalSignature,keyEncipherment");
    X509_add_ext(cert, ex, -1);
    X509_EXTENSION_free(ex);

    // Sign the certificate with our own key ("Self Sign")
    if (!X509_sign(cert, pk, EVP_sha1()))
    {
	seterr("Could not self sign the certificate");
	X509_free(cert);
	EVP_PKEY_free(pk);
	return;
    }
    debug("Certificate for %s created\n", dn);

    // Now that we have the certificate created,
    // be nice and leave it in enccert, so if someone needs it, they
    // don't have to do that step... I'm not sure that this is going
    // to stay... it's really not that hard to call encodecert() ;)
    // It's staying... no need to make things harder than necessary ;)
    encodecert();
}


WvString WvX509Mgr::createcertreq(WvString dName, int keysize)
{
    EVP_PKEY *pk;
    X509_NAME *name = NULL;
    X509_REQ *certreq;
    WvString pkcs10("");
    struct stat stupidstat;

    FILE *stupidtmp = tmpfile();

    // First thing to do is to generate an RSA Keypair if the
    // Manager doesn't already have one:
    if (rsa == NULL)
	rsa = new WvRSAKey(keysize);
    
    if ((pk=EVP_PKEY_new()) == NULL)
    {
        seterr("Error creating key handler for new certificate");
        return pkcs10;
    }
    
    if ((certreq=X509_REQ_new()) == NULL)
    {
        seterr("Error creating new PKCS#10 object");
        return pkcs10;
    }

    if (!EVP_PKEY_assign_RSA(pk, rsa->rsa))
    {
        seterr("Error adding RSA keys to certificate");
        return pkcs10;
    }
    
    X509_REQ_set_pubkey(certreq, pk);
    name = X509_REQ_get_subject_name(certreq);   

    set_name_entry(name, dName);
    
    X509_REQ_set_subject_name(certreq, name);

    // Horribly involuted hack to get around the fact that the
    // OpenSSL people are too braindead to have a PEM_write function
    // that returns a char *
    PEM_write_X509_REQ(stupidtmp,certreq);
    // With any luck, PEM_write won't close the file ;)
  
    rewind(stupidtmp);

    fstat(fileno(stupidtmp),&stupidstat);

    pkcs10.setsize(stupidstat.st_size + 1);

    fread(pkcs10.edit(),sizeof(char),stupidstat.st_size,stupidtmp);

    fclose(stupidtmp);

    pkcs10.edit()[stupidstat.st_size] = 0;
    X509_REQ_free(certreq);
    EVP_PKEY_free(pk);

    return pkcs10;
}


bool WvX509Mgr::testcert()
{
    bool bad = false;
    
    EVP_PKEY *pk = EVP_PKEY_new();
    
    if (rsa && pk)
    {
	WvRSAKey tmpkey(*rsa);
	
	if (!EVP_PKEY_assign_RSA(pk, tmpkey.rsa))
    	{
            seterr("Error setting RSA keys");
	    bad = true;
    	}
	else
	{
	    int verify_return = X509_verify(cert, pk);
	    if (verify_return != 1) // only '1' means okay
	    {
		seterr("Certificate test failed: %s\n", wvssl_errstr());
		bad = true;
	    }
	}
	
	tmpkey.rsa = NULL;
    }
    else
    {
	seterr("no RSA keypair in X509 manager");
	bad = true;
    }
    
    if (pk)
	EVP_PKEY_free(pk);
    
    return !bad;
}


void WvX509Mgr::decodecert(WvString encodedcert)
{
    int hexbytes = strlen((const char *)encodedcert);
    int bufsize = hexbytes/2;
    unsigned char *certbuf = new unsigned char[bufsize];
    const unsigned char *cp = certbuf;
    X509 *tmpcert;

    unhexify(certbuf,encodedcert);
    tmpcert = cert = X509_new();
    cert = wv_d2i_X509(&tmpcert, &cp, hexbytes/2);

    // make sure that the cert is valid
    if (cert && !testcert())
    {
	X509_free(cert);
	cert = NULL;
    }
    
    if (!cert)
	seterr("certificate decode failed!");
    
    delete[] certbuf;
}


void WvX509Mgr::encodecert()
{
    size_t size;
    unsigned char *keybuf, *iend;

    size = i2d_X509(cert, NULL);
    iend = keybuf = new unsigned char[size];
    i2d_X509(cert, &iend);

    enccert.setsize(size * 2 +1);
    hexify(enccert.edit(), keybuf, size);

    delete[] keybuf;
}


bool WvX509Mgr::validate()
{
    if (cert != NULL)
    {
	debug("Peer Certificate:\n");
	debug("SubjectDN: %s\n",
	      X509_NAME_oneline(X509_get_subject_name(cert),0,0));
	debug("Issuer: %s\n",
	      X509_NAME_oneline(X509_get_issuer_name(cert),0,0));

	// Check and make sure that the certificate is still valid
	if (X509_cmp_current_time(X509_get_notAfter(cert)) == -1)
	{
	    seterr("Peer certificate has expired!");
	    return false;
	}
	
	// Kind of a placeholder thing right now...
	// Later on, do CRL, and certificate validity checks here..
        // Actually, break these out in signedbyvalidCA(), and isinCRL()
	// Maybe have them here and activated by bool values as parameters 
	// to validate.
    }
    else
	debug("Peer doesn't have a certificate.\n");
    
    return true;
}


bool WvX509Mgr::signedbyCAinfile(WvString certfile)
{
    X509_STORE *cert_ctx = NULL;
    X509_STORE_CTX csc;
    X509_LOOKUP *lookup = NULL;
    int result = 0;

    cert_ctx = X509_STORE_new();
    if (cert_ctx == NULL)
    {
	seterr("Unable to create Certificate Store Context");
	return false;
    }

    lookup=X509_STORE_add_lookup(cert_ctx,X509_LOOKUP_file());
    if (lookup == NULL) abort();  

    if (!X509_LOOKUP_load_file(lookup,certfile,X509_FILETYPE_PEM))
        X509_LOOKUP_load_file(lookup,NULL,X509_FILETYPE_DEFAULT);

    X509_STORE_CTX_init(&csc,cert_ctx,cert,NULL);
    result = X509_verify_cert(&csc);
    X509_STORE_CTX_cleanup(&csc);
    
    X509_STORE_free(cert_ctx);

    if (result == 1)
    	return true;
    else
	return false;
}


bool WvX509Mgr::signedbyCAindir(WvString certdir)
{
    WvDirIter i(certdir,false);
    for (i.rewind(); i.next() ; )
    {
	if (!signedbyCAinfile(i->fullname))
	    return false;
    }    
    return true;
}


bool WvX509Mgr::isinCRL()
{
    return true;
}


void WvX509Mgr::load(WvString hexified_cert, WvString hexified_rsa)
{
    if (rsa)
        delete rsa;
    if (cert)
	delete cert;

    rsa = new WvRSAKey(hexified_rsa, true);
    if (!rsa->isok())
    {
	seterr("RSA Error: %s\n", rsa->errstring);
	return;
    }

    decodecert(hexified_cert);

    // Don't handle errors here, because the caller should just check 
    // isok() when load is finished anyways.

    return;
}


void WvX509Mgr::dump(WvString filename, DumpMode mode, bool append)
{
    FILE *outfile;
    const EVP_CIPHER *enc;
    
    if (append)
    {
	debug("Opening %s for append.\n", filename);
	outfile = fopen(filename, "a");
    }
    else
    {
	debug("Opening %s for write.\n", filename);
	outfile = fopen(filename, "w");
    }
    
    if (outfile)
    {
	switch(mode)
	{
	case CertPEM:
	    debug("Dumping X509 Certificate.\n");
	    PEM_write_X509(outfile, cert);
	    break;
	case RsaPEM:
	    debug("Dumping RSA keypair.\n");
	    enc = EVP_get_cipherbyname("rsa");
	    PEM_write_RSAPrivateKey(outfile, rsa->rsa, enc, NULL, 0, NULL, NULL);
	    break;
	case RsaRaw:
	    debug("Dumping raw RSA keypair.\n");
	    RSA_print_fp(outfile, rsa->rsa, 0);
	    break;
	default:
	    seterr("Unknown Mode\n");
	}
	fclose(outfile);
    }
    else
        seterr("fopen: %s", strerror(errno));
}
