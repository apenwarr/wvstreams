/*
 * Insert Appropriate Copyright header here....
 * Also, the license should specify that it is LGPL ;)
 * ppatterson@carillonis.com
 */

#include "wvx509.h"

#include "pem.h"
#include "x509v3.h"

 
WvX509Mgr::WvX509Mgr(X509 *_cert)
	: debug("X509",WvLog::Debug5),
	  errstr("")
{
   err = false;
   cert = _cert;
   keypair = NULL;
}

WvX509Mgr::WvX509Mgr(WvString dName, int bits, WvRSAKey *_keypair)
	: debug("X509",WvLog::Debug5),
	  errstr("")
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

void WvX509Mgr::createSScert(WvString dName, int keysize)
{
        EVP_PKEY *pk;
        X509_NAME *name=NULL;
        X509_EXTENSION *ex=NULL;
	int	serial = 12345; // RFC2459 says that this number
				// must be unique for each certificate
				// issued by a CA - since this is a
				// self-signed cert, we'll take a shortcut,
				// and give a fixed value... saves a couple
				// of cycles rather than get a random number.
	WvString serverfqdn;	
	int	name_err;

	if (keypair == NULL)
	{
	    debug("Need a new RSA Key, so generating it...\n");
	    keypair = new WvRSAKey(keysize);
	    debug("Ok, I've got a new RSA Keypair\n");
	}

	if ((pk=EVP_PKEY_new()) == NULL)
        {
	    err = true;
            errstr = "Error Creating Key Handler for new Certificate";  
            return;
        }
        if ((cert=X509_new()) == NULL)
        {
	    err = true;
	    errstr = "Error Creating new X509 object";
	    return;
        }
	if (!EVP_PKEY_assign_RSA(pk,keypair->rsa))
        {
	    err = true;
            errstr = "Error Adding RSA Keys to Certificate";
            return;
        }

	// Completely broken in my mind - this sets the version
	// string to '3'  (I guess version starts at 0)
	X509_set_version(cert,0x2);

	// Set the Serial Number for the certificate
        ASN1_INTEGER_set(X509_get_serialNumber(cert),serial);

	// Set the NotBefore time to now.
        X509_gmtime_adj(X509_get_notBefore(cert),0);

	// Now + 10 years... should be shorter, but since we don't currently
	// Have a set of routines to refresh the certificates, make it
	// REALLY long.
        X509_gmtime_adj(X509_get_notAfter(cert),(long)60*60*24*3650);
        X509_set_pubkey(cert,pk);

        name=X509_get_subject_name(cert);

	WvStringList list;
	WvStringList::Iter i(list);
	list.zap();
	list.split(dName,",");
	for (i.rewind() ; i.next() ; )
	{
		WvString data;
		char *servname;
		// This entire conditional serves no purpose other than
		// set up serverfqdn for an extension entry later....
		// There's probably a way to do this without needing the
		// extra logic here, but this was the part of the day
		// "Where Patrick learns WvStrings, and gets to practice
		// with the neat ways to assign things to them" :)
		if (strncmp(*i,"cn=",3) == 0)
		{
		    if ((servname = strchr(*i,'=')) != NULL)
		    {
			serverfqdn = servname+1;
			serverfqdn.unique();
		    }
		    else
		    {
			// Theoretically, we should never get here...
			err = true;
			errstr = "Malformed CN Entry... Aborting...";
		    	X509_free(cert);
		    	EVP_PKEY_free(pk);
			return;
		    }	
		}
		else
		{
		   if (!serverfqdn)
			serverfqdn = "null.weavernet.null";
		}
		char *cptr = strchr(i->edit(),'=');
		if (cptr)
		{
		    data = cptr + 1;
		    *cptr = 0;
		}
		else
		{
		    err = true;
		    errstr =  "Illegal DN During Certificate Creation";
		    X509_free(cert);
		    EVP_PKEY_free(pk);
		    return;
		}
		// The strupr() calls here are to workaround some REALLY
		// stupid bits in the OpenSSL code that make it so that
		// the add_entry_by_txt call fails if the contents of both
		// data and the iterator are not upper case... Why they
		// chose this arbitrary restriction is beyond me. 
		unsigned char *temp = (unsigned char *)strupr(data.edit());
		name_err = X509_NAME_add_entry_by_txt(name, strupr(i->edit()), 
				MBSTRING_ASC,temp, -1 , -1 , 0);

		if (name_err == 0)
		{
		    err = true;
		    errstr = "Error creating Name entry... aborting Certificate Creation";
		    X509_free(cert);
		    EVP_PKEY_free(pk);
		    return;
		}
	}

	X509_set_issuer_name(cert,name);
	X509_set_subject_name(cert,name);

	// Add in the Netscape specific Server Extension
	ex = X509V3_EXT_conf_nid(NULL, NULL, NID_netscape_cert_type, "server");
        X509_add_ext(cert,ex,-1);
        X509_EXTENSION_free(ex);

	debug("Setting Netscape SSL Server Name Extension to %s\n",serverfqdn);

	// Set the Netscape Server Name Extension to our server name
	ex = X509V3_EXT_conf_nid(NULL, NULL, NID_netscape_ssl_server_name, serverfqdn.edit());
        X509_add_ext(cert,ex,-1);
        X509_EXTENSION_free(ex);

	// Set the RFC2459 Mandated keyUsage field to critical, and restrict
	// the usage of this cert to Digital Signature, and Key Encipherment.
	ex = X509V3_EXT_conf_nid(NULL, NULL, NID_key_usage, "critical,digitalSignature,keyEncipherment");
	X509_add_ext(cert,ex,-1);
	X509_EXTENSION_free(ex);

	// Sign the Certificate with our own key ("Self Sign")
        if (!X509_sign(cert,pk,EVP_md5()))
        {
	    err = true;
	    errstr = "Could not self sign the certificate";
	    X509_free(cert);
	    EVP_PKEY_free(pk);
	    return;
	}
	debug("Certificate for %s created\n",dName);

	// Now that we have the certificate created,
	// be nice and leave it in enccert, so if someone needs it, they
	// don't have to do that step... I'm not sure that this is going 
	// to stay... it's really not that hard to call encodecert() ;) 
	encodecert();

	return;
}

void WvX509Mgr::decodecert(WvString *encodedcert)
{
    	int hexbytes = strlen((const char *)encodedcert);
    	int bufsize = hexbytes/2;
	unsigned char *certbuf = new unsigned char[bufsize];
	X509 *ct;

	unhexify(certbuf,(char *)encodedcert);
	ct = cert = X509_new();
	cert = d2i_X509(&ct, &certbuf, hexbytes/2);

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

	return;
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
		err = true;
		errstr = "Certificate past it's validity period";
		return false;
	    }
	    // Kind of a placeholder thing right now...
	    // Later on, do CRL, and certificate validity checks here..
	} else {
	    debug("Connection Peer not using a certificate\n");
	}
	return true;	
}