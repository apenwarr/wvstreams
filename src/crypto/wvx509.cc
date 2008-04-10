/*
 * Insert Appropriate Copyright header here....
 * Also, the license should specify that it is LGPL ;)
 * ppatterson@carillonis.com
 */

#include "wvx509.h"
#include "wvdiriter.h"

#include "pem.h"
#include "x509v3.h"

WvX509Mgr::WvX509Mgr(X509 *_cert)
    : debug("X509",WvLog::Debug5), errstr("")
{
    err = false;
    cert = _cert;
    keypair = NULL;
}

WvX509Mgr::WvX509Mgr(WvString dName, int bits, WvRSAKey *_keypair)
    : debug("X509",WvLog::Debug5), errstr("")
{
    debug("Creating New Certificate for %s\n",dName);
    keypair = _keypair;
    cert = NULL;
    err = false;
    createSScert(dName, bits);
}


WvX509Mgr::~WvX509Mgr()
{
    X509_free(cert);
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
	    force_fqdn.unique();
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
	    force_fqdn.unique();
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

    if (keypair == NULL)
    {
	debug("Need a new RSA key, so generating it...\n");
	keypair = new WvRSAKey(keysize);
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
    if (!EVP_PKEY_assign_RSA(pk, keypair->rsa))
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
    if (!X509_sign(cert, pk, EVP_md5()))
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
    if ( keypair == NULL)
    {
	keypair = new WvRSAKey(keysize);
    }
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

    if (!EVP_PKEY_assign_RSA(pk, keypair->rsa))
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

void WvX509Mgr::decodecert(WvString encodedcert)
{
    int hexbytes = strlen((const char *)encodedcert);
    int bufsize = hexbytes/2;
    unsigned char *certbuf = new unsigned char[bufsize], *cp = certbuf;
    X509 *ct;

    unhexify(certbuf,encodedcert);
    ct = cert = X509_new();
    cert = d2i_X509(&ct, (const unsigned char **)&cp, hexbytes/2);

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
    {
        X509_LOOKUP_load_file(lookup,NULL,X509_FILETYPE_DEFAULT);
    }

    X509_STORE_CTX_init(&csc,cert_ctx,cert,NULL);
    result = X509_verify_cert(&csc);
    X509_STORE_CTX_cleanup(&csc);
    
    X509_STORE_free(cert_ctx);

    if (result == 1)
    {
    	return true;
    }
    else
    {
	return false;
    }
}

bool WvX509Mgr::signedbyCAindir(WvString certdir)
{
    WvDirIter i(certdir,false);
    for (i.rewind(); i.next() ; )
    {
	if (!signedbyCAinfile(i->fullname))
	{
	    return false;
	}
    }    
    return true;
}

bool WvX509Mgr::isinCRL()
{
    return true;
}

void WvX509Mgr::dumpcert(WvString outfile, bool append)
{
    FILE *certout;

    if (append)
    {
	certout = fopen(outfile,"a");
        debug("Opening %s for append\n",outfile);
    }
    else
    {
	certout = fopen(outfile,"w");
        debug("Opening %s for write\n",outfile);
    }

    if (certout != NULL)
    {
	debug("Dumping X509 Certificate...\n");
    	PEM_write_X509(certout,cert);
    }
    else
    {
	seterr("Cannot open file for writing");
    }

    fclose(certout);

    return;
}

void WvX509Mgr::dumpkeypair(WvString outfile, bool append)
{
    FILE *keyout;
    EVP_CIPHER *enc = NULL;

    if (append)
    {
        keyout = fopen(outfile,"a");
        debug("Opening %s for append\n",outfile);
    }
    else
    {
        keyout = fopen(outfile,"w");
    }
    if (keyout != NULL)
    {
	debug("Printing keypair...\n");
	enc = (EVP_CIPHER *)EVP_get_cipherbyname("rsa");
	PEM_write_RSAPrivateKey(keyout,keypair->rsa, enc, NULL, 0, NULL, NULL);
    }
    else
    {   
        seterr("Cannot open file for writing");
    }

    fclose(keyout);

    return;
}

void WvX509Mgr::dumprawkeypair(WvString outfile, bool append)
{
    FILE *keyout;
    int offset;
    struct stat filestat;

    if (append)
    {
        keyout = fopen(outfile,"a");
        fstat(fileno(keyout),&filestat);
	offset = filestat.st_size;
        debug("Opening %s for append\n",outfile);
    }
    else
    {
        keyout = fopen(outfile,"w");
	offset = 0;
    }
    if (keyout != NULL)
    {
	debug("Printing keypair...\n");
	RSA_print_fp(keyout,keypair->rsa, offset);
    }
    else
    {   
        seterr("Cannot open file for writing");
    }

    fclose(keyout);

    return;
}
