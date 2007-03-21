 /*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2005 Net Integration Technologies, Inc.
 * 
 * X.509 certificate management classes.
 */ 
#include "wvx509.h"
#include "wvrsa.h"
#include "wvcrl.h"
#include "wvsslhacks.h"
#include "wvdiriter.h"
#include "wvcrypto.h"
#include "wvstringlist.h"
#include "wvbase64.h"
#include "wvstrutils.h"
#include "wvfileutils.h"

#include <openssl/pem.h>
#include <openssl/x509v3.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/sha.h>
#include <openssl/pkcs12.h>

UUID_MAP_BEGIN(WvX509Mgr)
  UUID_MAP_ENTRY(IObject)
  UUID_MAP_END

static int ssl_init_count = 0;

namespace {
class AutoClose {
public:
    AutoClose(FILE *fp): fp(fp) { }
    ~AutoClose()
    {
        if (fp)
            fclose(fp);
    }
    
    operator FILE *() const
    {
	return fp;
    }
    
private:
    FILE *fp;
};
} // anomymous namespace...



void wvssl_init()
{
    if (!ssl_init_count)
    {
	SSL_library_init();
	SSL_load_error_strings();
	ERR_load_BIO_strings();
	ERR_load_crypto_strings();
	OpenSSL_add_all_algorithms();
	OpenSSL_add_all_ciphers();
	OpenSSL_add_all_digests();
    }
    
    ssl_init_count++;
}


void wvssl_free()
{
    if (ssl_init_count >= 1)
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
    : debug("X509", WvLog::Debug5), pkcs12pass(WvString::null)
{
    wvssl_init();
    cert = _cert;
    rsa = NULL;
    if (cert)
    {
	filldname();
	rsa = fillRSAPubKey();
	if (!rsa->isok())
	    seterr("RSA public key error: %s", rsa->errstr());
    }
    else
	;
	// This isn't an error, or a mistake...
	// so this is one case where 
	// cert == NULL && rsa == NULL && errstr == NULL
	// That the programmer should be doing something about.
}


WvX509Mgr::WvX509Mgr()
    : debug("X509", WvLog::Debug5), pkcs12pass(WvString::null)
{
    wvssl_init();
    cert = NULL;
    rsa = NULL;
}


void WvX509Mgr::load(DumpMode mode, WvStringParm fname)
{
    if (mode == CertASN1)
    {
        BIO *bio = BIO_new(BIO_s_file());
        if (BIO_read_filename(bio, fname.cstr()) <= 0)
        {
            BIO_free(bio);
            seterr(errno);
            return;
        }

        if (!(cert = d2i_X509_bio(bio, NULL)))
        {
            BIO_free(bio);       
            seterr("Can't read certificate from file");
            return;
        }

        BIO_free(bio);

        if (!rsa)
        {
            rsa = fillRSAPubKey();
            // the RSA key does not always need to be valid. we could have
            // a DSA key instead?
            //if (!rsa->isok())
            //    seterr("RSA public key error: %s", rsa->errstr());
        }
    }
    else
    {
        WvDynBuf buffer;    
        WvFile f(fname, O_RDONLY);    
        while (f.isok())
        {
            f.read(buffer, 1000);
        }
        f.close();
        decode(mode, buffer.getstr());
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
	seterr("RSA error: %s", rsa->errstr());
	return;
    }

    if (!!hexified_cert)
	unhexify(hexified_cert);
    else
    {
	seterr("No hexified certificate");
	return;
    }

    if (cert)
	filldname();
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
    rsa = NULL;
    
    if (!!dname)
    {
	rsa = new WvRSAKey(bits);
	create_selfsigned();
    }
    else
	seterr("Sorry, can't create an anonymous certificate");
}


WvX509Mgr::~WvX509Mgr()
{
    debug("Deleting.\n");
    
    WVDELETE(rsa);

    if (cert)
	X509_free(cert);

    wvssl_free();
}


bool WvX509Mgr::bind_ssl(SSL_CTX *ctx)
{
    if (SSL_CTX_use_certificate(ctx, cert) <= 0)
    {
	return false;
    }
    debug("Certificate activated.\n");
    
    if (SSL_CTX_use_RSAPrivateKey(ctx, rsa->rsa) <= 0)
    {
	return false;
    }
    debug("RSA private key activated.\n");
    return true;
}


const WvRSAKey &WvX509Mgr::get_rsa()
{
    assert(rsa);

    return *rsa;
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
static WvString set_name_entry(X509_NAME *name, WvStringParm dn)
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
	else if (sid == "email")
	    nid = NID_pkcs9_emailAddress;
	else
	    nid = NID_domainComponent;
	
	// Sometimes we just want to parse dn into fqdn.
	if (name == NULL)
	    continue;
	
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

    return fqdn;
}


void WvX509Mgr::create_selfsigned(bool is_ca)
{
    assert(rsa);

    if (cert)
    {
	debug("Replacing already existant certificate...\n");
	X509_free(cert);
	cert = NULL;
    }

    // double check RSA key
    if (rsa->isok())
	debug("RSA Key is fine.\n");
    else
    {
	seterr("RSA Key is bad");
	return;
    }

    if ((cert = X509_new()) == NULL)
    {
	seterr("Error creating new X509 object");
	return;
    }

    // Completely broken in my mind - this sets the version
    // string to '3'  (I guess version starts at 0)
    set_version();

    // RFC2459 says that this number must be unique for each certificate
    // issued by a CA.  It may be that some web browsers get confused if
    // more than one cert with the same name has the same serial number, so
    // let's be careful.
    srand(time(NULL));
    int	serial = rand();
    set_serial(serial);
    
    // 10 years...
    set_lifetime(60*60*24*3650);
    
    set_pubkey(rsa);
				       
    set_issuer(dname);
    set_subject(dname);
    
    if (is_ca)
    {
	debug("Setting Extensions with CA Parameters.\n");
	debug("Setting Key Usage.\n");
	set_key_usage("critical, keyCertSign, cRLSign");
	debug("Setting Basic Constraints.\n");
	set_extension(NID_basic_constraints, "critical, CA:TRUE");
	debug("Setting Netscape Certificate Type.\n");
	set_extension(NID_netscape_cert_type, "SSL CA, S/MIME CA, Object Signing CA");
//	debug("Setting Constraints.\n");
//	set_constraints("requireExplicitPolicy");
    }
    else
    {
	debug("Setting Key Usage with normal server parameters\n");
	set_nsserver(dname);
	set_key_usage("critical, digitalSignature, keyEncipherment, keyAgreement");
	set_extension(NID_basic_constraints, "CA:FALSE");
	set_ext_key_usage("TLS Web Server Authentication,"
			  "TLS Web Client Authentication");
    }
    
    debug("Ok - Parameters set... now signing certificate.\n");
    signcert(cert);
    
    debug("Certificate for %s created\n", dname);
}


void WvX509Mgr::filldname()
{
    assert(cert);
    
    char *name = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
    dname = name;
    OPENSSL_free(name);
}


// FIXME: This is EVIL!!!
WvRSAKey *WvX509Mgr::fillRSAPubKey()
{
    EVP_PKEY *pkcert = X509_get_pubkey(cert);
    RSA *certrsa = EVP_PKEY_get1_RSA(pkcert);
    EVP_PKEY_free(pkcert);
    return new WvRSAKey(certrsa, false); 
}


WvString WvX509Mgr::certreq()
{
    EVP_PKEY *pk = NULL;
    X509_NAME *name = NULL;
    X509_REQ *certreq = NULL;

    assert(rsa);
    assert(dname);

    // double check RSA key
    if (rsa->isok())
	debug("RSA Key is fine.\n");
    else
    {
	seterr("RSA Key is bad");
	return WvString::null;
    }

    if ((pk=EVP_PKEY_new()) == NULL)
    {
        seterr("Error creating key handler for new certificate");
        return WvString::null;
    }
    
    if ((certreq=X509_REQ_new()) == NULL)
    {
        seterr("Error creating new PKCS#10 object");
	EVP_PKEY_free(pk);
        return WvString::null;
    }

    if (!EVP_PKEY_set1_RSA(pk, rsa->rsa))
    {
        seterr("Error adding RSA keys to certificate");
	X509_REQ_free(certreq);
	EVP_PKEY_free(pk);
        return WvString::null;
    }
    
    X509_REQ_set_version(certreq, 0); /* version 1 */

    X509_REQ_set_pubkey(certreq, pk);

    name = X509_REQ_get_subject_name(certreq);

    debug("Creating Certificate request for %s\n", dname);
    set_name_entry(name, dname);
    X509_REQ_set_subject_name(certreq, name);
    char *sub_name = X509_NAME_oneline(X509_REQ_get_subject_name(certreq), 
				       0, 0);
    debug("SubjectDN: %s\n", sub_name);
    OPENSSL_free(sub_name);
    
    if (!X509_REQ_sign(certreq, pk, EVP_sha1()))
    {
	seterr("Could not self sign the request");
	X509_REQ_free(certreq);
	EVP_PKEY_free(pk);
        return WvString::null;
    }

    int verify_result = X509_REQ_verify(certreq, pk);
    if (verify_result == 0)
    {
	seterr("Self signed request failed");
	X509_REQ_free(certreq);
	EVP_PKEY_free(pk);
        return WvString::null;
    }
    else
    {
	debug("Self Signed Certificate Request verifies OK!\n");
    }

    // Horribly involuted hack to get around the fact that the
    // OpenSSL people are too braindead to have a PEM_write function
    // that returns a char *
    WvDynBuf retval;
    BIO *bufbio = BIO_new(BIO_s_mem());
    BUF_MEM *bm;
    
    PEM_write_bio_X509_REQ(bufbio, certreq);
    BIO_get_mem_ptr(bufbio, &bm);
    retval.put(bm->data, bm->length);
    
    X509_REQ_free(certreq);
    EVP_PKEY_free(pk);
    BIO_free(bufbio);

    return retval.getstr();
}


WvString WvX509Mgr::signreq(WvStringParm pkcs10req)
{
    assert(rsa);
    assert(cert);
    debug("Signing a certificate request with : %s\n", get_subject());
    
    // Break this next part out into a de-pemify section, since that is what
    // this part up until the FIXME: is about.
    WvString pkcs10(pkcs10req);
    
    char *begin = strstr(pkcs10.edit(), "\nMII");
    if (!begin)
    {
	debug("This doesn't look like PEM Encoded information...\n");
	return WvString::null;
    }
    char *end = strstr(begin + 1, "\n---");
    if (!end)
    {
	debug("Is this a complete certificate request?\n");
	return WvString::null;
    }
    *++end = '\0';
    WvString body(begin); // just the PKCS#10 request, 
                          // without the ---BEGIN and ---END
    
    WvDynBuf reqbuf;
    WvBase64Decoder dec;
    dec.flushstrbuf(body, reqbuf, true);
    
    // FIXME: Duplicates code from cert_selfsign
    size_t reqlen = reqbuf.used();
    const unsigned char *req = reqbuf.get(reqlen);
    X509_REQ *certreq = wv_d2i_X509_REQ(NULL, &req, reqlen);
    if (certreq)
    {
	WvX509Mgr newcert;

	newcert.set_subject(X509_REQ_get_subject_name(certreq));
	newcert.set_version();
	
	// Set the Serial Number for the certificate
	srand(time(NULL));
	int serial = rand();
	newcert.set_serial(serial);
	
	newcert.set_lifetime(60*60*24*3650);
	
	// The public key of the new cert should be the same as that from 
	// the request.
	EVP_PKEY *pk = X509_REQ_get_pubkey(certreq);
	X509_set_pubkey(newcert.get_cert(), pk);
	EVP_PKEY_free(pk);
	
	// The Issuer name is the subject name of the current cert
	newcert.set_issuer(get_subject());
	
	X509_EXTENSION *ex = NULL;
	// Set the RFC2459-mandated keyUsage field to critical, and restrict
	// the usage of this cert to digital signature and key encipherment.
	newcert.set_key_usage("critical, digitalSignature, keyEncipherment");
    
	// This could cause Netscape to barf because if we set basicConstraints 
	// to critical, we break RFC2459 compliance. Why they chose to enforce 
	// that bit, and not the rest is beyond me... but oh well...
	ex = X509V3_EXT_conf_nid(NULL, NULL, NID_basic_constraints,
				 "CA:FALSE");
	
	X509_add_ext(newcert.get_cert(), ex, -1);
	X509_EXTENSION_free(ex);

	newcert.set_ext_key_usage("critical, TLS Web Client Authentication");

	signcert(newcert.get_cert());
	
	X509_REQ_free(certreq);
	return WvString(newcert.encode(CertPEM));
    }
    else
    {
	debug("Can't decode Certificate Request\n");
	return WvString::null;
    }
}


bool WvX509Mgr::test()
{
    bool bad = false;
    
    EVP_PKEY *pk = EVP_PKEY_new();
    
    if (!cert)
    {
	seterr("No certificate in X509 manager");
	bad = true;
    }
    
    if (rsa && pk)
    {
	if (!EVP_PKEY_set1_RSA(pk, rsa->rsa))
    	{
            seterr("Error setting RSA keys");
	    bad = true;
    	}
	else if (!bad)
	{
	    int verify_return = X509_verify(cert, pk);
	    if (verify_return != 1) // only '1' means okay
	    {
		// However let's double check:
		WvString rsapub = encode(RsaPubPEM);
		WvRSAKey *temprsa = fillRSAPubKey();
		WvString certpub = temprsa->getpem(false);
		delete temprsa;
		// debug("rsapub:\n%s\n", rsapub);
		// debug("certpub:\n%s\n", certpub);
		if (certpub == rsapub)
		    ; // do nothing, since OpenSSL is lying
		else
		{
		    // I guess that it really did fail.
		    seterr("Certificate test failed: %s\n", wvssl_errstr());
		    bad = true;
		}
	    }
	}
    }
    else
    {
	seterr("No RSA keypair in X509 manager");
	bad = true;
    }
    
    if (pk)
	EVP_PKEY_free(pk);
    
    return !bad;
}


void WvX509Mgr::unhexify(WvStringParm encodedcert)
{
    if (!encodedcert)
    {
	seterr("X509 certificate can't be decoded from nothing");
	return;
    }
    
    int hexbytes = strlen(encodedcert.cstr());
    int bufsize = hexbytes/2;
    unsigned char *certbuf = new unsigned char[bufsize];
    unsigned char *cp = certbuf;
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
	seterr("X509 certificate decode failed");
    
    deletev certbuf;
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

    deletev keybuf;
    return enccert;
}


bool WvX509Mgr::validate(WvX509Mgr *cacert, X509_CRL *crl)
{
    bool retval = true;
    
    if (cert != NULL)
    {
	// Check and make sure that the certificate is still valid
	if (X509_cmp_current_time(X509_get_notAfter(cert)) == -1)
	{
	    seterr("Certificate has expired");
	    retval = false;
	}
	
	if (cacert)
	    retval &= signedbyCA(cacert);

	// Kind of a placeholder thing right now...
	// Later on, do CRL, and certificate validity checks here..
        // Actually, break these out in signedbyvalidCA(), and isinCRL()
	// Maybe have them here and activated by bool values as parameters 
	// to validate.
    }
    else
	debug("Peer doesn't have a certificate.\n");
    
    return retval;
}


bool WvX509Mgr::signedbyCA(WvX509Mgr *cacert)
{
    int ret = X509_check_issued(cacert->cert, cert);
    debug("signedByCA: %s==X509_V_OK(%s)\n", ret, X509_V_OK);
    if (ret == X509_V_OK)    
	return true;
    else
	return false;
}


WvString WvX509Mgr::encode(const DumpMode mode)
{
    WvString nil;
    WvDynBuf retval;
    BIO *bufbio = BIO_new(BIO_s_mem());
    BUF_MEM *bm;
    
    switch(mode)
    {
    case CertPEM:
	debug("Dumping X509 certificate.\n");
	PEM_write_bio_X509(bufbio, cert);
	break;
	
    case CertDER:
	debug("Dumping X509 certificate in DER format\n");
	i2d_X509_bio(bufbio, cert);
	break;
	
    case RsaPEM:
	debug("Dumping RSA keypair.\n");
	BIO_free(bufbio);
	return rsa->getpem(true);
	break;
	
    case RsaPubPEM:
	debug("Dumping RSA Public Key!\n");
	BIO_free(bufbio);
	return rsa->getpem(false);
	break;

    case RsaRaw:
	debug("Dumping raw RSA keypair.\n");
	RSA_print(bufbio, rsa->rsa, 0);
	break;
	
    default:
	seterr(EINVAL);
	return nil;
    }

    BIO_get_mem_ptr(bufbio, &bm);
    retval.put(bm->data, bm->length);
    BIO_free(bufbio);
    if (mode == CertDER)
    {
	WvBase64Encoder enc;
	WvString output;
	enc.flushbufstr(retval, output, true);
	return output;
    }
    else
	return retval.getstr();
}

void WvX509Mgr::decode(const DumpMode mode, WvStringParm encoded)
{
    if (!encoded)
    {
	debug(WvLog::Error, "Not decoding an empty string. - Sorry!\n");
	return;
    }

    BIO *membuf = BIO_new(BIO_s_mem());
    BIO_puts(membuf, encoded);
    
    switch(mode)
    {
    case CertPEM:
	debug("Importing X509 certificate.\n");
	if(cert)
	{
	    debug("Replacing an already existant X509 Certificate!\n");
	    X509_free(cert);
	    cert = NULL;
	}
	
	cert = PEM_read_bio_X509(membuf, NULL, NULL, NULL);
	if (cert)
	{
	    filldname();
	    if (!rsa)
		rsa = fillRSAPubKey();
	}
	else
	    seterr("Certificate failed to import");
	break;
    case RsaPEM:
	debug("Importing RSA keypair.\n");
	debug("Make sure that you load or generate a new Certificate!\n");
	WVDELETE(rsa);
	
	rsa = new WvRSAKey(PEM_read_bio_RSAPrivateKey(membuf, NULL, NULL, NULL), 
			   true);
	if (!rsa->isok())
	    seterr("RSA key failed to import");
	break;
    case RsaPubPEM:
	debug("Importing RSA Public Key.\n");
	debug("Are you REALLY sure that you want to do this?\n");
	if (rsa) delete rsa;
	rsa = new WvRSAKey(PEM_read_bio_RSAPublicKey(membuf, NULL, NULL, NULL), 
			   true);
	if (!rsa->isok())
	    seterr("RSA public key failed to import");
	break;
    case RsaRaw:
	debug("Importing raw RSA keypair not supported.\n");
	break;
	
    default:
	seterr(EINVAL);
    }
    BIO_free_all(membuf);
}


void WvX509Mgr::write_p12(WvStringParm filename)
{
    debug("Dumping RSA Key and X509 Cert to PKCS12 structure.\n");

    AutoClose fp = fopen(filename, "wb");

    if (!fp)
    {
        seterr(errno);
        return;
    }

    if (!!pkcs12pass)
    {
	if (rsa && cert)
	{
	    EVP_PKEY *pk = EVP_PKEY_new();
	    if (!pk)
	    {
		seterr("Unable to create PKEY object");
		return;
	    }

	    if (!EVP_PKEY_set1_RSA(pk, rsa->rsa))
	    {
		seterr("Error setting RSA keys");
		EVP_PKEY_free(pk);
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
		    EVP_PKEY_free(pk);
		}
		else
		{
		    seterr("Unable to create PKCS12 object");
		    EVP_PKEY_free(pk);
		    return;
		}
	    }
	}
	else
	{
	    seterr("Either the RSA key or the certificate is not present");
	    return;
	}
    }
    else
    {
        seterr("No password specified for PKCS12 dump");
        return; 
    }
}

void WvX509Mgr::read_p12(WvStringParm filename)
{
    debug("Reading Certificate and Private Key from PKCS12 file: %s\n", filename);

    AutoClose fp = fopen(filename, "r");

    if (!fp)
    {
        seterr(errno);
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
		seterr("Unable to create PKEY object");
		return;
	    }
	    
	    // Parse out the bits out the PKCS12 package.
	    PKCS12_parse(pkg, pkcs12pass, &pk, &cert, NULL);
	    PKCS12_free(pkg);
	    
	    // Now, cert should be OK, let's try and set up the RSA stuff
	    // since we've essentially got a PKEY, and not a WvRSAKey
	    // We need to create a new WvRSAKey from the PKEY...
	    rsa = new WvRSAKey(EVP_PKEY_get1_RSA(pk), true);
	    
	    // Now that we have both, check to make sure that they match
	    if (!rsa || !cert || test())
	    {
		seterr("Could not fill in RSA and certificate with matching values");
		return;
	    }
	    EVP_PKEY_free(pk);
	}
	else
	{
	    seterr("Read in of PKCS12 file '%s' failed", filename);
	    return;
	}
    }
    else
    {
        seterr("No password specified for PKCS12 file");
        return;
    }
}


WvString WvX509Mgr::get_issuer()
{ 
    if (cert)
    {
	char *name = X509_NAME_oneline(X509_get_issuer_name(cert),0,0);
        WvString retval(name);
        OPENSSL_free(name);
	return retval;
    }
    else
	return WvString::null;
}


void WvX509Mgr::set_issuer(WvStringParm issuer)
{
    assert(cert);
    X509_NAME *name = X509_get_issuer_name(cert);
    set_name_entry(name, issuer);
    X509_set_issuer_name(cert, name);
}


WvString WvX509Mgr::get_subject()
{
    if (cert)
    {
	char *name = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
	WvString retval(name);
	OPENSSL_free(name);
	return retval;
    }
    else
	return WvString::null;
}


void WvX509Mgr::set_subject(WvStringParm subject)
{
    assert(cert);
    X509_NAME *name = X509_get_subject_name(cert);
    set_name_entry(name, subject);
    X509_set_subject_name(cert, name);
}


void WvX509Mgr::set_subject(X509_NAME *name)
{
    X509_set_subject_name(cert, name);
}


void WvX509Mgr::set_pubkey(WvRSAKey *_rsa)
{
    EVP_PKEY *pk = NULL;

    if ((pk = EVP_PKEY_new()) == NULL)
    {
	seterr("Error creating key handler for new certificate");
	return;
    }

    // Assign RSA Key from WvRSAKey into stupid package that OpenSSL needs
    if (!EVP_PKEY_set1_RSA(pk, rsa->rsa))
    {
	seterr("Error adding RSA keys to certificate");
	return;
    }
    
    X509_set_pubkey(cert, pk);

    if (pk)
	EVP_PKEY_free(pk);
}



void WvX509Mgr::set_nsserver(WvStringParm servername)
{
    assert(cert);
    
    WvString fqdn;
    
    // FQDN cannot have a = in it, therefore it
    // must be a distinguished name :)
    if (strchr(servername, '='))
	fqdn = set_name_entry(NULL, servername);
    else
	fqdn = servername;
    
    if (!fqdn)
	fqdn = "null.noname.null";
    
    debug("Setting Netscape SSL server name extension to '%s'.\n", fqdn);

    // Add in the netscape-specific server extension
    set_extension(NID_netscape_cert_type, "server");
    set_extension(NID_netscape_ssl_server_name, fqdn);
}


WvString WvX509Mgr::get_nsserver()
{
    return get_extension(NID_netscape_ssl_server_name);
}


WvString WvX509Mgr::get_serial()
{
    if (cert)
    {
	return WvString(ASN1_INTEGER_get(X509_get_serialNumber(cert)));
    }
    else
	return WvString::null;
}


void WvX509Mgr::set_version()
{
	X509_set_version(cert, 0x2);
}


void WvX509Mgr::set_serial(long serial)
{
    assert(cert);
    ASN1_INTEGER_set(X509_get_serialNumber(cert), serial);
}


WvString WvX509Mgr::get_crl_dp()
{
    assert(cert);
    return get_extension(NID_crl_distribution_points);
}


WvString WvX509Mgr::get_cp_oid()
{
    assert(cert);
    return get_extension(NID_certificate_policies);
}

void WvX509Mgr::set_cp_oid(WvStringParm oid, WvStringParm _url)
{
    assert(cert);
    WvString url(_url);
    ASN1_OBJECT *pobj = OBJ_txt2obj(oid, 0);
    POLICYINFO *pol = POLICYINFO_new();
    POLICYQUALINFO *qual = NULL;
    STACK_OF(POLICYINFO) *sk_pinfo = sk_POLICYINFO_new_null();
    pol->policyid = pobj;
    if (!!url)
    {
	pol->qualifiers = sk_POLICYQUALINFO_new_null();
	qual = POLICYQUALINFO_new();
	qual->pqualid = OBJ_nid2obj(NID_id_qt_cps);
	qual->d.cpsuri = M_ASN1_IA5STRING_new();
	ASN1_STRING_set(qual->d.cpsuri, url.edit(), url.len());
	sk_POLICYQUALINFO_push(pol->qualifiers, qual);
    }
    sk_POLICYINFO_push(sk_pinfo, pol);
    X509_EXTENSION *ex = X509V3_EXT_i2d(NID_certificate_policies, 0, 
					sk_pinfo);
    X509_add_ext(cert, ex, -1);
    X509_EXTENSION_free(ex);
    sk_POLICYINFO_free(sk_pinfo);
}


void WvX509Mgr::set_lifetime(long seconds)
{
    // Set the NotBefore time to now.
    X509_gmtime_adj(X509_get_notBefore(cert), 0);
    
    // Now + 10 years... should be shorter, but since we don't currently
    // have a set of routines to refresh the certificates, make it
    // REALLY long.
    X509_gmtime_adj(X509_get_notAfter(cert), seconds);
}


void WvX509Mgr::set_key_usage(WvStringParm values)
{
    set_extension(NID_key_usage, values);
}


WvString WvX509Mgr::get_key_usage()
{
    return get_extension(NID_key_usage);
}


void WvX509Mgr::set_ext_key_usage(WvStringParm values)
{
    set_extension(NID_ext_key_usage, values);
}


WvString WvX509Mgr::get_ext_key_usage()
{
    assert(cert);
    return get_extension(NID_ext_key_usage);
}


WvString WvX509Mgr::get_altsubject()
{
    assert(cert);
    return get_extension(NID_subject_alt_name);
}


WvString WvX509Mgr::get_constraints()
{
    assert(cert);
    return get_extension(NID_policy_constraints);
}


void WvX509Mgr::set_constraints(WvStringParm constraint)
{
    assert(cert);
    set_extension(NID_policy_constraints, constraint);
}


void WvX509Mgr::set_aia(WvStringParm _identifier)
{
    WvString identifier(_identifier);
    unsigned char *list;
    list = reinterpret_cast<unsigned char *>(identifier.edit());
    AUTHORITY_INFO_ACCESS *ainfo = sk_ACCESS_DESCRIPTION_new_null();
    ACCESS_DESCRIPTION *acc = ACCESS_DESCRIPTION_new();
    sk_ACCESS_DESCRIPTION_push(ainfo, acc);
    GENERAL_NAME_free(acc->location);
    i2d_GENERAL_NAME(acc->location, &list);
#if OPENSSL_VERSION_NUMBER >= 0x0090800fL
    const unsigned char** plist = const_cast<const unsigned char**>(&list);
#else
    unsigned char** plist = &list;
#endif
    d2i_GENERAL_NAME(&acc->location, plist, identifier.len());
    X509_EXTENSION *ex = X509V3_EXT_i2d(NID_info_access, 0, ainfo);
    X509_add_ext(cert, ex, -1);
    X509_EXTENSION_free(ex);
    sk_ACCESS_DESCRIPTION_free(ainfo);
}


WvString WvX509Mgr::get_aia()
{
    return get_extension(NID_info_access);
}


WvStringList *parse_stack(WvStringParm ext, 
			 WvStringList *list, WvStringParm prefix)
{
    WvStringList whole_aia;
    whole_aia.split(ext, "\n");
    WvStringList::Iter i(whole_aia);
    for (i.rewind();i.next();)
    {
      WvString stack_entry(*i);
      if (strstr(stack_entry, prefix))
      {
          WvString uri(stack_entry.edit() + prefix.len());
          list->append(uri);  
      }
    }
    return list;
}

WvStringList *WvX509Mgr::get_ocsp(WvStringList *responders)
{
    return parse_stack(get_aia(), responders, "OCSP - URI:");
}


WvStringList *WvX509Mgr::get_ca_urls(WvStringList *urls)
{
    return parse_stack(get_aia(), urls, "CA Issuers - URI:");
}


WvString WvX509Mgr::get_extension(int nid)
{
    WvString retval = WvString::null;
    
    if (cert)
    {
	int index = X509_get_ext_by_NID(cert, nid, -1);
	if (index >= 0)
	{
	    X509_EXTENSION *ext = X509_get_ext(cert, index);

	    if (ext)
	    {
		X509V3_EXT_METHOD *method = X509V3_EXT_get(ext);
		if (!method)
		{
		    WvDynBuf buf;
		    buf.put(ext->value->data, ext->value->length);
		    retval = buf.getstr();
		}
		else
		{
		    void *ext_data = NULL;
                    // we NEED to use a temporary pointer for ext_value_data,
                    // as openssl's ASN1_item_d2i will muck around with it, 
                    // even though it's const (at least as of version 0.9.8e). 
                    // gah.
#if OPENSSL_VERSION_NUMBER >= 0x0090800fL
                    const unsigned char * ext_value_data = ext->value->data;
#else
                    unsigned char *ext_value_data = ext->value->data;
#endif
		    if (method->it)
		    {
 			ext_data = ASN1_item_d2i(NULL, &ext_value_data,
 						ext->value->length, 
 						ASN1_ITEM_ptr(method->it));
			debug("Applied generic conversion!\n");
		    }
		    else
		    {
			ext_data = method->d2i(NULL, &ext_value_data,
                                               ext->value->length);
			debug("Applied method specific conversion!\n");
		    }
		    
		    if (method->i2s)
		    {
			debug("String Extension!\n");
			retval = method->i2s(method, ext_data);
		    }
		    else if (method->i2v)
		    {
			debug("Stack Extension!\n");
		        CONF_VALUE *val = NULL;
		        STACK_OF(CONF_VALUE) *svals = NULL;
		        svals = method->i2v(method, ext_data, NULL);
		        if (!sk_CONF_VALUE_num(svals))
                            retval = "EMPTY";
                        else
                        {
                            WvStringList list;
                            for(int i = 0; i < sk_CONF_VALUE_num(svals); i++)
                            {
                                val = sk_CONF_VALUE_value(svals, i);
                                if (!val->name)
                                    list.append(WvString(val->value));
                                else if (!val->value)
                                    list.append(WvString(val->name));
                                else 
                                {
                                    WvString pair("%s:%s", val->name, val->value);
                                    list.append(pair);
                                }
                            }
                            retval = list.join(";\n");
                        }
                        sk_CONF_VALUE_pop_free(svals, X509V3_conf_free);
		    }
		    else if (method->i2r)
		    {
			debug("Raw Extension!\n");
			WvDynBuf retvalbuf;
			BIO *bufbio = BIO_new(BIO_s_mem());
			BUF_MEM *bm;
			method->i2r(method, ext_data, bufbio, 0);
			BIO_get_mem_ptr(bufbio, &bm);
			retvalbuf.put(bm->data, bm->length);
			BIO_free(bufbio);
			retval = retvalbuf.getstr();
		    }
		    
		    if (method->it)
			ASN1_item_free((ASN1_VALUE *)ext_data, 
				       ASN1_ITEM_ptr(method->it));
		    else
			method->ext_free(ext_data);

		}
	    }
	}
	else
	{
	    debug("Extension not present!\n");
	}

    }

    if (!!retval)
    {
	debug("Returning: %s\n", retval);
	return retval;
    }
    else
	return WvString::null;
}

void WvX509Mgr::set_extension(int nid, WvStringParm _values)
{
    WvString values(_values);
    X509_EXTENSION *ex = NULL;
    ex = X509V3_EXT_conf_nid(NULL, NULL, nid, values.edit());
    X509_add_ext(cert, ex, -1);
    X509_EXTENSION_free(ex);
}


bool WvX509Mgr::isok() const
{
    return cert && rsa && WvError::isok();
}


WvString WvX509Mgr::errstr() const
{
    if (WvError::geterr() == 0)
    {
        // only use a custom string if there's not an error set
        if (!cert && !rsa)
            return "No certificate or RSA key assigned";
        else if (!cert)
            return "No certificate assigned";
        else if (!rsa)
            return "No RSA key assigned";
    }
    return WvError::errstr();
}


int WvX509Mgr::geterr() const
{
    int ret = WvError::geterr();
    if (ret == 0 && (!cert || !rsa))
    {
        // unless there's a regular error set, we'll be returning a custom
        // string: see errstr()
        ret = -1;
    }
    return ret;
}


bool WvX509Mgr::signcert(X509 *unsignedcert)
{
    if (unsignedcert == NULL)
    {
	debug("No certificate to sign??\n");
	return false;
    }

    if (cert == unsignedcert)
    {
	debug("Self Signing!\n");
	printf("Looks like:\n%s\n", encode(WvX509Mgr::CertPEM).cstr());
    }
    else if (!((cert->ex_flags & EXFLAG_KUSAGE) && 
	  (cert->ex_kusage & KU_KEY_CERT_SIGN)))
    {
	debug("This Certificate is not allowed to sign Certificates!\n");
	return false;
    }
    
    debug("Ok, now sign the new cert with the current RSA key.\n");
    EVP_PKEY *certkey = EVP_PKEY_new();
    bool cakeyok = EVP_PKEY_set1_RSA(certkey, rsa->rsa);
    if (cakeyok)
    {
	
	X509_sign(unsignedcert, certkey, EVP_sha1());
    }
    else
    {
	debug("No keys??\n");
	EVP_PKEY_free(certkey);
	return false;
    }
    
    EVP_PKEY_free(certkey);
    return true;
}

WvString WvX509Mgr::sign(WvStringParm data)
{
    WvDynBuf buf;
    buf.putstr(data);
    return sign(buf);
}

WvString WvX509Mgr::sign(WvBuf &data)
{
    assert(rsa);

    EVP_MD_CTX sig_ctx;
    unsigned char sig_buf[4096];
    
    EVP_PKEY *pk = EVP_PKEY_new();
    if (!pk)
    {
	seterr("Unable to create PKEY object");
	return WvString::null;
    }
    
    if (!EVP_PKEY_set1_RSA(pk, rsa->rsa))
    {
	seterr("Error setting RSA keys");
	EVP_PKEY_free(pk);
	return WvString::null;
    }
    
    EVP_SignInit(&sig_ctx, EVP_sha1());
    EVP_SignUpdate(&sig_ctx, data.peek(0, data.used()), data.used());
    unsigned int sig_len = sizeof(sig_buf);
    int sig_err = EVP_SignFinal(&sig_ctx, sig_buf, 
				&sig_len, pk);
    if (sig_err != 1)
    {
	seterr("Error while signing");
	EVP_PKEY_free(pk);
	return WvString::null;
    }

    EVP_PKEY_free(pk);
    EVP_MD_CTX_cleanup(&sig_ctx); // this isn't my fault ://
    WvDynBuf buf;
    buf.put(sig_buf, sig_len);
    debug("Signature size: %s\n", buf.used());
    return WvBase64Encoder().strflushbuf(buf, true);
}

bool WvX509Mgr::verify(WvStringParm original, WvStringParm signature)
{
    WvDynBuf buf;
    buf.putstr(original);
    return verify(buf, signature);
}

bool WvX509Mgr::verify(WvBuf &original, WvStringParm signature)
{
    
    unsigned char sig_buf[4096];
    size_t sig_size = sizeof(sig_buf);
    WvBase64Decoder().flushstrmem(signature, sig_buf, &sig_size, true);
    
    EVP_PKEY *pk = X509_get_pubkey(cert);
    if (!pk) 
    {
        seterr("Couldn't allocate PKEY for verification");
        return false;
    }
    
    /* Verify the signature */
    EVP_MD_CTX sig_ctx;
    EVP_VerifyInit(&sig_ctx, EVP_sha1());
    EVP_VerifyUpdate(&sig_ctx, original.peek(0, original.used()), original.used());
    int sig_err = EVP_VerifyFinal(&sig_ctx, sig_buf, sig_size, pk);
    EVP_PKEY_free(pk);
    EVP_MD_CTX_cleanup(&sig_ctx); // Again, not my fault... 
    if (sig_err != 1) 
    {
        debug("Verify failed!\n");
        return false;
    }
    else
	return true;
}


time_t ASN1_TIME_to_time_t(ASN1_TIME *t)
{
    struct tm newtime;
    char *p = NULL;
    char d[18];
    memset(&d,'\0',sizeof(d));    
    memset(&newtime,'\0',sizeof newtime);
    
    if (t->type == V_ASN1_GENERALIZEDTIME) 
    {
         // For time values >= 2050, OpenSSL uses
         // ASN1_GENERALIZEDTIME - which we'll worry about
         // later.
	return 0;
    }

    p = (char *)t->data;
    sscanf(p,"%2s%2s%2s%2s%2s%2sZ", d, &d[3], &d[6], &d[9], &d[12], &d[15]);
    
    int year = strtol(d, (char **)NULL, 10);
    if (year < 49)
	year += 100;
    else
	year += 50;
    
    newtime.tm_year = year;
    newtime.tm_mon = strtol(&d[3], (char **)NULL, 10) - 1;
    newtime.tm_mday = strtol(&d[6], (char **)NULL, 10);
    newtime.tm_hour = strtol(&d[9], (char **)NULL, 10);
    newtime.tm_min = strtol(&d[12], (char **)NULL, 10);
    newtime.tm_sec = strtol(&d[15], (char **)NULL, 10);

    return mktime(&newtime);
}

time_t WvX509Mgr::get_notvalid_before()
{
    assert(cert);
    return ASN1_TIME_to_time_t(X509_get_notBefore(cert));
}


time_t WvX509Mgr::get_notvalid_after()
{
    assert(cert);
    return ASN1_TIME_to_time_t(X509_get_notAfter(cert));
}


bool WvX509Mgr::signcrl(WvCRLMgr *crl)
{
    assert(crl);
    assert(rsa);

    if (!((cert->ex_flags & EXFLAG_KUSAGE) && 
	  (cert->ex_kusage & KU_CRL_SIGN)))
    {
	debug("Certificate not allowed to sign CRLs!\n");
	return false;
    }
    
    EVP_PKEY *certkey = EVP_PKEY_new();
    bool cakeyok = EVP_PKEY_set1_RSA(certkey, rsa->rsa);
    if (crl->getcrl() && cakeyok)
    {
	// Use Version 2 CRLs - Of COURSE that means
	// to set it to 1 here... grumble..
	X509_CRL_set_version(crl->getcrl(), 1);

	X509_CRL_set_issuer_name(crl->getcrl(), X509_get_subject_name(cert));

	ASN1_TIME *tmptm = ASN1_TIME_new();
	// Set the LastUpdate time to now.
	X509_gmtime_adj(tmptm, 0);
	X509_CRL_set_lastUpdate(crl->getcrl(), tmptm);
	// CRL's are valid for 30 days
	X509_gmtime_adj(tmptm, (long)60*60*24*30);
	X509_CRL_set_nextUpdate(crl->getcrl(), tmptm);
	ASN1_TIME_free(tmptm);
	
	// OK - now sign it...
	X509_CRL_sign(crl->getcrl(), certkey, EVP_sha1());
    }
    else
    {
	debug("No keys??\n");
	EVP_PKEY_free(certkey);
	return false;
    }
    EVP_PKEY_free(certkey);

    return true;
}


WvString WvX509Mgr::get_ski()
{
    assert(cert);
    return get_extension(NID_subject_key_identifier);
}

WvString WvX509Mgr::get_aki()
{
    assert(cert);
    return get_extension(NID_authority_key_identifier);
}
