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

#include <openssl/pem.h>
#include <openssl/x509v3.h>
#include <openssl/err.h>
#include <openssl/sha.h>
#include <openssl/pkcs12.h>

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
    // HACK: never allow this function to actually free stuff, because I
    // think we're doing it wrong.
    if (ssl_init_count >= 2)
	ssl_init_count--;

    if (!ssl_init_count)
    {
	assert(0);
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
    : debug("X509", WvLog::Debug5), pkcs12pass(WvString::null)
{
    wvssl_init();
    cert = _cert;
    rsa = NULL;
    if (cert)
    {
	char buffer[1024];

	X509_NAME_oneline(X509_get_subject_name(cert), buffer, 1024);
	buffer[sizeof(buffer)-1] = 0;
	dname = buffer;
	
	EVP_PKEY *pk;
	pk = X509_get_pubkey(cert);

        if ((pk = X509_get_pubkey(cert)) != NULL)
	{
 	    size_t size;
            unsigned char *keybuf, *iend;
	    WvString tmppub;

	    size = i2d_RSAPublicKey(pk->pkey.rsa, NULL);
            iend = keybuf = new unsigned char[size];
            i2d_RSAPublicKey(pk->pkey.rsa, &iend);

            tmppub.setsize(size * 2 + 1);
	    ::hexify(tmppub.edit(), keybuf, size);
	    rsa = new WvRSAKey(tmppub,false);

	    delete[] keybuf;
	}
    }
}


WvX509Mgr::WvX509Mgr(WvStringParm hexified_cert,
		     WvStringParm hexified_rsa)
    : debug("X509", WvLog::Debug5), pkcs12pass(WvString::null)
{
    wvssl_init();
    
    cert = NULL;
    rsa = new WvRSAKey(hexified_rsa, true);
    if (!rsa->isok())
    {
	seterr("RSA Error: %s\n", rsa->errstr());
	return;
    }

    unhexify(hexified_cert);

    if (cert)
    {
	char buffer[1024];

    	X509_NAME_oneline(X509_get_subject_name(cert), buffer, 1024);
    	buffer[sizeof(buffer)-1] = 0;
    	dname = buffer;
    }
}


WvX509Mgr::WvX509Mgr(WvStringParm _dname, WvRSAKey *_rsa)
    : dname(_dname), debug("X509", WvLog::Debug5), pkcs12pass(WvString::null)
{
    assert(_rsa);
    
    wvssl_init();
    debug("Creating new certificate for %s\n", dname);
    cert = NULL;
    rsa = _rsa;
    create_selfsigned();
}


WvX509Mgr::WvX509Mgr(WvStringParm _dname, int bits)
    : dname(_dname), debug("X509", WvLog::Debug5)
{
    wvssl_init();
    debug("Creating new certificate for %s\n", dname);
    cert = NULL;
    rsa = new WvRSAKey(bits);
    create_selfsigned();
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
	else if (sid == "l")
	    nid = NID_localityName;
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


void WvX509Mgr::create_selfsigned()
{
    EVP_PKEY *pk;
    X509_NAME *name = NULL;
    X509_EXTENSION *ex = NULL;

    // RFC2459 says that this number must be unique for each certificate
    // issued by a CA.  It may be that some web browsers get confused if
    // more than one cert with the same name has the same serial number, so
    // let's be careful.
    srand(time(NULL));
    int	serial = random();

    WvString serverfqdn;

    assert(rsa);

    // double check RSA key
    if (RSA_check_key(rsa->rsa) == 1)
	debug("RSA Key is fine.\n");
    else
    {
	seterr("RSA Key is bad!\n");
	return;
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

    // Assign RSA Key from WvKey into stupid package that OpenSSL needs
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
    serverfqdn = set_name_entry(name, dname);
    
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
	     "critical,digitalSignature,keyEncipherment,keyCertSign");
    X509_add_ext(cert, ex, -1);
    X509_EXTENSION_free(ex);
    
#if 0 // this causes netscape to have a hairball
    ex = X509V3_EXT_conf_nid(NULL, NULL, NID_basic_constraints,
			     "critical, CA:FALSE");
    X509_add_ext(cert, ex, -1);
    X509_EXTENSION_free(ex);
#endif
    
    ex = X509V3_EXT_conf_nid(NULL, NULL, NID_ext_key_usage,
	     "TLS Web Server Authentication,TLS Web Client Authentication");
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

    debug("Certificate for %s created\n", dname);
}


static FILE *file_hack_start()
{
    return tmpfile();
}


static WvString file_hack_end(FILE *f)
{
    WvDynamicBuffer b;
    size_t len;
    
    rewind(f);
    while ((len = fread(b.alloc(1024), 1, 1024, f)) > 0)
	b.unalloc(1024 - len);
    b.unalloc(1024 - len);
    fclose(f);

    return b.getstr();
}


WvString WvX509Mgr::certreq()
{
    EVP_PKEY *pk;
    X509_NAME *name = NULL;
    X509_REQ *certreq;
    WvString nil;

    FILE *stupid = file_hack_start();
    
    assert(rsa);

    // double check RSA key
    if (RSA_check_key(rsa->rsa) == 1)
	debug("RSA Key is fine.\n");
    else
    {
	seterr("RSA Key is bad!\n");
	return nil;
    }

    if ((pk=EVP_PKEY_new()) == NULL)
    {
        seterr("Error creating key handler for new certificate");
        return nil;
    }
    
    if ((certreq=X509_REQ_new()) == NULL)
    {
        seterr("Error creating new PKCS#10 object");
	EVP_PKEY_free(pk);
        return nil;
    }

    WvRSAKey rsa2(*rsa);
    if (!EVP_PKEY_assign_RSA(pk, rsa2.rsa))
    {
        seterr("Error adding RSA keys to certificate");
	X509_REQ_free(certreq);
	EVP_PKEY_free(pk);
        return nil;
    }
    
    X509_REQ_set_version(certreq, 0); /* version 1 */

    X509_REQ_set_pubkey(certreq, pk);

    name = X509_REQ_get_subject_name(certreq);

    debug("Creating Certificate request for %s\n", dname);
    set_name_entry(name, dname);
    X509_REQ_set_subject_name(certreq, name);
    debug("SubjectDN: %s\n",
	      X509_NAME_oneline(X509_REQ_get_subject_name(certreq), 0, 0));

    if (!X509_REQ_sign(certreq, pk, EVP_sha1()))
    {
	seterr("Could not self sign the request");
	X509_REQ_free(certreq);
	EVP_PKEY_free(pk);
        return WvString("");
    }

    int verify_result = X509_REQ_verify(certreq, pk);
    if (verify_result == 0)
    {
	seterr("Self Signed Request failed!");
	X509_REQ_free(certreq);
	EVP_PKEY_free(pk);
        return WvString("");
    }
    else
    {
	debug("Self Signed Certificate Request verifies OK!\n");
    }

    // don't let rsa2 free its own RSA key... the certificate will do that
    // for us.
    rsa2.rsa = NULL;

    // Horribly involuted hack to get around the fact that the
    // OpenSSL people are too braindead to have a PEM_write function
    // that returns a char *
    PEM_write_X509_REQ(stupid, certreq);
    X509_REQ_free(certreq);
    EVP_PKEY_free(pk);
  
    return file_hack_end(stupid);
}
 

bool WvX509Mgr::test()
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


void WvX509Mgr::unhexify(WvString encodedcert)
{
    int hexbytes = strlen((const char *)encodedcert);
    int bufsize = hexbytes/2;
    unsigned char *certbuf = new unsigned char[bufsize];
    const unsigned char *cp = certbuf;
    X509 *tmpcert;
    
    if (cert)
	X509_free(cert);

    ::unhexify(certbuf, encodedcert);
    tmpcert = cert = X509_new();
    cert = wv_d2i_X509(&tmpcert, &cp, hexbytes/2);

    // make sure that the cert is valid
    if (cert && !test())
    {
	X509_free(cert);
	cert = NULL;
    }
    
    if (!cert)
	seterr("certificate decode failed!");
    
    delete[] certbuf;
}


WvString WvX509Mgr::hexify()
{
    size_t size;
    unsigned char *keybuf, *iend;
    WvString enccert;

    size = i2d_X509(cert, NULL);
    iend = keybuf = new unsigned char[size];
    i2d_X509(cert, &iend);

    enccert.setsize(size * 2 +1);
    ::hexify(enccert.edit(), keybuf, size);

    delete[] keybuf;
    return enccert;
}


bool WvX509Mgr::validate()
{
    if (cert != NULL)
    {
	debug("Peer Certificate:\n");
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
    for (i.rewind(); i.next(); )
    {
	if (!signedbyCAinfile(i->fullname))
	    return false;
    }    
    return true;
}


bool WvX509Mgr::isinCRL()
{
    return false;
}


WvString WvX509Mgr::encode(DumpMode mode)
{
    FILE *stupid;
    const EVP_CIPHER *enc;
    WvString nil;
    
    stupid = file_hack_start();
    
    if (stupid)
    {
	switch(mode)
	{
	case CertPEM:
	    debug("Dumping X509 certificate.\n");
	    PEM_write_X509(stupid, cert);
	    break;
	    
	case RsaPEM:
	    debug("Dumping RSA keypair.\n");
	    enc = EVP_get_cipherbyname("rsa");
	    PEM_write_RSAPrivateKey(stupid, rsa->rsa, enc,
				    NULL, 0, NULL, NULL);
	    break;
	    
	case RsaRaw:
	    debug("Dumping raw RSA keypair.\n");
	    RSA_print_fp(stupid, rsa->rsa, 0);
	    break;

	default:
	    seterr("Unknown Mode\n");
	    return nil;
	}
	
	return file_hack_end(stupid);
    }
    else
    {
	debug(WvLog::Error, "Can't create temp file in WvX509Mgr::encode!\n");
	return nil;
    }
}

void WvX509Mgr::decode(DumpMode mode)
{
    // Let the fun begin... ;)

}

void WvX509Mgr::write_p12(WvStringParm filename)
{
    debug("Dumping RSA Key and X509 Cert to PKCS12 structure\n");

    FILE *fp = fopen(filename, "w");
    
    if (!fp)
    {
	seterr("Unable to create: %s\n", filename);
	return;
    }
    
    if (!!pkcs12pass)
    {
	EVP_PKEY *pk = EVP_PKEY_new();
	if (!pk)
	{
	    seterr("Unable to create PKEY object\n");
	    return;
	}
	
	if (rsa && cert)
	{
	    WvRSAKey tmpkey(*rsa);
	    
	    if (!EVP_PKEY_assign_RSA(pk, tmpkey.rsa))
	    {
		seterr("Error setting RSA keys");
		return;
	    }
	    else
	    {
		PKCS12 *pkg = PKCS12_create(pkcs12pass.edit(), "foo", pk, 
					    cert, NULL, 0, 0, 0, 0, 0);
		if (pkg)
		{
		    debug("Write the PKCS12 object out...\n");
		    i2d_PKCS12_fp(fp, pkg);
		    PKCS12_free(pkg);
		}
		else
		{
		    seterr("Unable to create PKCS12 object\n");
		    return;
		}
	    }
	}
	else
	{
	    seterr("Either the RSA key or the Certificate is not present\n");
	    return;
	}
    }
    else
    {
	seterr("No Password specified for PKCS12 dump\n");
	return;
    }
}

void WvX509Mgr::read_p12(WvStringParm filename)
{
    debug("Reading Certificate and Private Key from PKCS12 file: %s\n", filename);
    
    FILE *fp = fopen(filename, "r");
    
    if (!fp)
    {
	seterr("Unable to read from: %s\n", filename);
	return;
    }
    
    if (!!pkcs12pass)
    {
	PKCS12 *pkg = d2i_PKCS12_fp(fp, NULL);
	if (pkg)
	{
	    EVP_PKEY *pk = EVP_PKEY_new();
	    if (!pk)
	    {
		seterr("Unable to create PKEY object\n");
		return;
	    }
	    
	    // Parse out the bits out the PKCS12 package.
	    PKCS12_parse(pkg, pkcs12pass, &pk, &cert, NULL);
	    PKCS12_free(pkg);
	    
	    // Now, cert should be OK, let's try and set up the RSA stuff
	    // since we've essentially got a PKEY, and not a WvRSAKey
	    // We need to create a new WvRSAKey from the PKEY...
	    rsa = new WvRSAKey(pk->pkey.rsa, true);
	    
	    // Now that we have both, check to make sure that they match
	    if (!rsa || !cert || test())
	    {
		seterr("Could not fill in RSA and Cert with matching values... \n");
		return;
	    }
	}
	else
	{
	    seterr("Read in of PKCS12 file '%s' failed - aborting\n", filename);
	    return;
	}
    }
    else
    {
	seterr("No Password specified for PKCS12 file - aborting\n");
	return;	
    }
}