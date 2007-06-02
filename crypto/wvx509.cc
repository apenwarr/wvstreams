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
    : debug("X509", WvLog::Debug5)
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
    : debug("X509", WvLog::Debug5)
{
    wvssl_init();
    cert = NULL;
    rsa = NULL;
}


void WvX509Mgr::load(DumpMode mode, WvStringParm fname)
{
    if (mode == CertDER)
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
    : debug("X509", WvLog::Debug5)
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
    : dname(_dname), debug("X509", WvLog::Debug5)
{
    assert(_rsa);
    
    wvssl_init();
    debug("Creating new certificate for %s\n", dname);
    cert = NULL;
    rsa = _rsa;
    create_selfsigned();
}


WvX509Mgr::WvX509Mgr(WvStringParm _dname, int bits, bool isca)
    : dname(_dname), debug("X509", WvLog::Debug5)
{
    wvssl_init();
    debug("Creating new certificate for %s\n", dname);
    cert = NULL;
    rsa = NULL;
    
    if (!!dname)
    {
	rsa = new WvRSAKey(bits);
	create_selfsigned(isca);
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
    set_ski();

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


WvString WvX509Mgr::certreq(WvStringParm subject, const WvRSAKey &rsa)
{
    WvLog debug("X509::certreq", WvLog::Debug5);

    EVP_PKEY *pk = NULL;
    X509_NAME *name = NULL;
    X509_REQ *certreq = NULL;

    // double check RSA key
    if (rsa.isok())
	debug("RSA Key is fine.\n");
    else
    {
	debug(WvLog::Warning, "RSA Key is bad");
	return WvString::null;
    }

    if ((pk=EVP_PKEY_new()) == NULL)
    {
        debug(WvLog::Warning, "Error creating key handler for new certificate");
        return WvString::null;
    }
    
    if ((certreq=X509_REQ_new()) == NULL)
    {
        debug(WvLog::Warning, "Error creating new PKCS#10 object");
	EVP_PKEY_free(pk);
        return WvString::null;
    }

    if (!EVP_PKEY_set1_RSA(pk, rsa.rsa))
    {
        debug(WvLog::Warning, "Error adding RSA keys to certificate");
	X509_REQ_free(certreq);
	EVP_PKEY_free(pk);
        return WvString::null;
    }
    
    X509_REQ_set_version(certreq, 0); /* version 1 */

    X509_REQ_set_pubkey(certreq, pk);

    name = X509_REQ_get_subject_name(certreq);

    debug("Creating Certificate request for %s\n", subject);
    set_name_entry(name, subject);
    X509_REQ_set_subject_name(certreq, name);
    char *sub_name = X509_NAME_oneline(X509_REQ_get_subject_name(certreq), 
				       0, 0);
    debug("SubjectDN: %s\n", sub_name);
    OPENSSL_free(sub_name);
    
    if (!X509_REQ_sign(certreq, pk, EVP_sha1()))
    {
	debug(WvLog::Warning, "Could not self sign the request");
	X509_REQ_free(certreq);
	EVP_PKEY_free(pk);
        return WvString::null;
    }

    int verify_result = X509_REQ_verify(certreq, pk);
    if (verify_result == 0)
    {
	debug(WvLog::Warning, "Self signed request failed");
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
        newcert.cert = X509_new();

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

        // every good cert needs an ski+aki
        newcert.set_ski();
        newcert.set_aki(*this);

	// The Issuer name is the subject name of the current cert
	newcert.set_issuer(dname); // FIXME: get_subject gives bad results...
	
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


bool WvX509Mgr::validate(WvX509Mgr *cacert, WvCRL *crl)
{
    bool retval = true;
    
    if (cert != NULL)
    {
	// Check and make sure that the certificate is still valid
	if (X509_cmp_current_time(X509_get_notAfter(cert)) < 0)
	{
	    seterr("Certificate has expired.");
	    retval = false;
	}
	
        if (X509_cmp_current_time(X509_get_notBefore(cert)) > 0)
        {
            seterr("Certificate is not yet valid.");
            retval = false;
        }

	if (cacert)
        {
	    retval &= signedbyca(cacert);
            retval &= issuedbyca(cacert);
        }
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


bool WvX509Mgr::signedbyca(WvX509Mgr *cacert)
{
    EVP_PKEY *pkey = X509_get_pubkey(cacert->cert);
    int result = X509_verify(cert, pkey);    
    EVP_PKEY_free(pkey);

    if (result < 0)
    {
        debug("There was an error determining whether or not we were signed by "
              "CA '%s'\n", cacert->get_subject());
        return false;
    }
    bool issigned = (result > 0);

    debug("Certificate was%s signed by CA %s\n", issigned ? "" : " NOT", 
          cacert->get_subject());

    return issigned;
}


bool WvX509Mgr::issuedbyca(WvX509Mgr *cacert)
{
    int ret = X509_check_issued(cacert->cert, cert);
    debug("issuedbyca: %s==X509_V_OK(%s)\n", ret, X509_V_OK);
    if (ret != X509_V_OK)
	return false;

    return true;
}


WvString WvX509Mgr::encode(const DumpMode mode)
{
    WvString nil;
    WvDynBuf retval;
    encode(mode, retval);
    return retval.getstr();
}


void WvX509Mgr::encode(const DumpMode mode, WvBuf &buf)
{
    BIO *bufbio = BIO_new(BIO_s_mem());
    BUF_MEM *bm;
    
    switch(mode)
    {
    case CertPEM:
	debug("Dumping X509 certificate in PEM format.\n");
	PEM_write_bio_X509(bufbio, cert);
	break;
	
    case CertDER:
	debug("Dumping X509 certificate in DER format\n");
	i2d_X509_bio(bufbio, cert);
	break;
	
    case RsaPEM:
	debug("Dumping RSA keypair.\n");
	BIO_free(bufbio);
        buf.putstr(rsa->getpem(true));
        return;
	break;
	
    case RsaPubPEM:
	debug("Dumping RSA Public Key!\n");
	BIO_free(bufbio);
        buf.putstr(rsa->getpem(false));
        return;
	break;

    case RsaRaw:
	debug("Dumping raw RSA keypair.\n");
	RSA_print(bufbio, rsa->rsa, 0);
	break;
	
    default:
        // unacceptable! the programmer is doing something really dumb...
        assert(0 && "Tried to dump certificate in invalid format!");
	BIO_free(bufbio);
	return;
        break;
    }

    BIO_get_mem_ptr(bufbio, &bm);
    buf.put(bm->data, bm->length);
    BIO_free(bufbio);
}


void WvX509Mgr::decode(const DumpMode mode, WvStringParm encoded)
{
    if (!encoded)
    {
	debug(WvLog::Error, "Not decoding an empty string. - Sorry!\n");
	return;
    }
    
    WvDynBuf buf;
    buf.putstr(encoded);
    decode(mode, buf);
}


void WvX509Mgr::decode(const DumpMode mode, WvBuf &encoded)
{
    BIO *membuf = BIO_new(BIO_s_mem());
    BIO_write(membuf, encoded.get(encoded.used()), encoded.used());
    
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
    case CertDER:
	debug("Importing X509 certificate.\n");
	if (cert)
	{
	    debug("Replacing an already existant X509 Certificate!\n");
	    X509_free(cert);
	    cert = NULL;
	}
	
	cert = d2i_X509_bio(membuf, NULL);
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
        assert(0 && "Tried to import certificate in invalid format!");
        break;
    }
    BIO_free_all(membuf);

}


bool WvX509Mgr::write_p12(WvStringParm _fname, WvStringParm _pkcs12pass)
{
    debug("Dumping RSA Key and X509 Cert to PKCS12 structure.\n");

    AutoClose fp = fopen(_fname, "wb");

    if (!fp)
    {
        debug(WvLog::Warning, "Unable to open file. Error: %s\n", strerror(errno));
        return false;
    }

    if (!!_pkcs12pass)
    {
	if (rsa && cert)
	{
	    EVP_PKEY *pk = EVP_PKEY_new();
	    if (!pk)
	    {
		debug(WvLog::Warning, "Unable to create PKEY object.");
		return false;
	    }

	    if (!EVP_PKEY_set1_RSA(pk, rsa->rsa))
	    {
		seterr("Error setting RSA keys.");
		EVP_PKEY_free(pk);
		return false;
	    }
	    else
	    {
                WvString pkcs12pass(_pkcs12pass);
		PKCS12 *pkg = PKCS12_create(pkcs12pass.edit(), "foo", pk, 
					    cert, NULL, 0, 0, 0, 0, 0);
		if (pkg)
		{
		    debug("Writing the PKCS12 object out...\n");
		    i2d_PKCS12_fp(fp, pkg);
		    PKCS12_free(pkg);
		    EVP_PKEY_free(pk);
		}
		else
		{
		    debug(WvLog::Warning, "Unable to create PKCS12 object.");
		    EVP_PKEY_free(pk);
		    return false;
		}
	    }
	}
	else
	{
	    debug(WvLog::Warning, "Either the RSA key or the certificate is not present.");
	    return false;
	}
    }
    else
    {
        debug(WvLog::Warning, "No password specified for PKCS12 dump.");
        return false; 
    }

    return true;
}


void WvX509Mgr::read_p12(WvStringParm _fname, WvStringParm _pkcs12pass)
{
    debug("Reading Certificate and Private Key from PKCS12 file: %s\n", _fname);

    AutoClose fp = fopen(_fname, "r");

    if (!fp)
    {
        seterr(errno);
        return;
    }

    if (!!_pkcs12pass)
    {
	PKCS12 *pkg = d2i_PKCS12_fp(fp, NULL);
	if (pkg)
	{
	    EVP_PKEY *pk = NULL;
	    
	    // Parse out the bits out the PKCS12 package.
	    PKCS12_parse(pkg, _pkcs12pass, &pk, &cert, NULL);
	    PKCS12_free(pkg);
            if (!pk || !cert)
            {
                seterr("Could not decode pkcs12 file");
                EVP_PKEY_free(pk);
                return;
            }

	    // Now, cert should be OK, let's try and set up the RSA stuff
	    // since we've essentially got a PKEY, and not a WvRSAKey
	    // We need to create a new WvRSAKey from the PKEY...
	    rsa = new WvRSAKey(EVP_PKEY_get1_RSA(pk), true);
            EVP_PKEY_free(pk);

	    // Now that we have both, check to make sure that they match
	    if (!rsa || !cert || !test())
	    {
		seterr("Could not fill in RSA and certificate with matching values");
		return;
	    }
	}
	else
	{
	    seterr("Read in of PKCS12 file '%s' failed", _fname);
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
        BIGNUM *bn = BN_new();
        bn = ASN1_INTEGER_to_BN(X509_get_serialNumber(cert), bn);
        char * c = BN_bn2dec(bn);
        WvString ret("%s", c);
        OPENSSL_free(c);
        BN_free(bn);
	return ret;
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


bool WvX509Mgr::get_basic_constraints(bool &ca, int &pathlen)
{
    assert(cert);
    BASIC_CONSTRAINTS *constraints = NULL;
    int i;

    constraints = static_cast<BASIC_CONSTRAINTS *>(X509_get_ext_d2i(
                                                       cert, NID_basic_constraints,
                                                       &i, NULL));
    if (constraints)
    {
        ca = constraints->ca;
        if (constraints->pathlen)
        {
            if ((constraints->pathlen->type == V_ASN1_NEG_INTEGER) || !ca)
            {
                debug("Path length type not valid when getting basic constraints.\n");
                BASIC_CONSTRAINTS_free(constraints);
                pathlen = 0;
                return false;
            }
            
            pathlen = ASN1_INTEGER_get(constraints->pathlen);
        }
        else
            pathlen = (-1);

        BASIC_CONSTRAINTS_free(constraints);
        return true;
    }
    
    debug("Basic constraints extension not present.\n");
    return false;
}


void WvX509Mgr::set_basic_constraints(bool ca, int pathlen)
{
    assert(cert);
    BASIC_CONSTRAINTS *constraints = BASIC_CONSTRAINTS_new();
    
    constraints->ca = static_cast<int>(ca);
    if (pathlen != (-1))
    {
        ASN1_INTEGER *i = ASN1_INTEGER_new();
        ASN1_INTEGER_set(i, pathlen);
        constraints->pathlen = i;
    }

    X509_EXTENSION *ex = X509V3_EXT_i2d(NID_basic_constraints, 0, 
                                        constraints);
    while (int idx = X509_get_ext_by_NID(cert, NID_basic_constraints, 0) >= 0)
    {
        debug("Found extension at idx %s\n", idx);
        X509_EXTENSION *tmpex = X509_delete_ext(cert, idx);
        X509_EXTENSION_free(tmpex);
    }

    X509_add_ext(cert, ex, NID_basic_constraints);
    X509_EXTENSION_free(ex);
    BASIC_CONSTRAINTS_free(constraints);
}


bool WvX509Mgr::get_policy_constraints(int &require_explicit_policy, 
                                       int &inhibit_policy_mapping)
{
    assert(cert);
    POLICY_CONSTRAINTS *constraints = NULL;
    int i;
    
    constraints = static_cast<POLICY_CONSTRAINTS *>(X509_get_ext_d2i(
                                                cert, NID_policy_constraints, 
                                                &i, NULL));
    if (constraints)
    {
        if (constraints->requireExplicitPolicy)
            require_explicit_policy = ASN1_INTEGER_get(
                constraints->requireExplicitPolicy);
        else
            require_explicit_policy = (-1);

        if (constraints->inhibitPolicyMapping)
            inhibit_policy_mapping = ASN1_INTEGER_get(
                constraints->inhibitPolicyMapping);
        else
            inhibit_policy_mapping = (-1);
        POLICY_CONSTRAINTS_free(constraints);
        return true;
    }

    return false;
}


void WvX509Mgr::set_policy_constraints(int require_explicit_policy, 
                                       int inhibit_policy_mapping)
{
    assert(cert);
    POLICY_CONSTRAINTS *constraints = POLICY_CONSTRAINTS_new();
    
    ASN1_INTEGER *i = ASN1_INTEGER_new();
    ASN1_INTEGER_set(i, require_explicit_policy);
    constraints->requireExplicitPolicy = i;
    i = ASN1_INTEGER_new();
    ASN1_INTEGER_set(i, inhibit_policy_mapping);
    constraints->inhibitPolicyMapping = i;

    X509_EXTENSION *ex = X509V3_EXT_i2d(NID_policy_constraints, 0, 
                                        constraints);
    X509_add_ext(cert, ex, -1);
    X509_EXTENSION_free(ex);
    POLICY_CONSTRAINTS_free(constraints);
}


bool WvX509Mgr::get_policy_mapping(PolicyMapList &list)
{
    assert(cert);
    POLICY_MAPPINGS *mappings = NULL;
    POLICY_MAPPING *map = NULL;
    int i;

    mappings = static_cast<POLICY_MAPPINGS *>(X509_get_ext_d2i(
                                                cert, NID_policy_mappings, 
                                                &i, NULL));
    if (!mappings)
        return false;

    const int POLICYID_MAXLEN = 80;
    char tmp1[80];
    char tmp2[80];
    for(int j = 0; j < sk_POLICY_MAPPING_num(mappings); j++) 
    {
        map = sk_POLICY_MAPPING_value(mappings, j);
        OBJ_obj2txt(tmp1, POLICYID_MAXLEN, map->issuerDomainPolicy, true);
        OBJ_obj2txt(tmp2, POLICYID_MAXLEN, map->subjectDomainPolicy, true);
        list.append(new PolicyMap(tmp1, tmp2), true);
    }

    sk_POLICY_MAPPING_pop_free(mappings, POLICY_MAPPING_free);
    
    return true;
}


void WvX509Mgr::set_policy_mapping(PolicyMapList &list)
{
    assert(cert);
    POLICY_MAPPINGS *maps = sk_POLICY_MAPPING_new_null();
    
    PolicyMapList::Iter i(list);
    for (i.rewind(); i.next();)
    {
        POLICY_MAPPING *map = POLICY_MAPPING_new();
        map->issuerDomainPolicy = OBJ_txt2obj(i().issuer_domain.cstr(), 0);
        map->subjectDomainPolicy = OBJ_txt2obj(i().subject_domain.cstr(), 0);
        sk_POLICY_MAPPING_push(maps, map);
        printf("Push!\n");
    }

    X509_EXTENSION *ex = X509V3_EXT_i2d(NID_policy_mappings, 0, maps);
    X509_add_ext(cert, ex, -1);
    X509_EXTENSION_free(ex);
    sk_POLICY_MAPPING_pop_free(maps, POLICY_MAPPING_free);
}


static void add_aia(WvStringParm type, WvString identifier, AUTHORITY_INFO_ACCESS *ainfo)
{
    ACCESS_DESCRIPTION *acc = ACCESS_DESCRIPTION_new();
    sk_ACCESS_DESCRIPTION_push(ainfo, acc);
    acc->method = OBJ_txt2obj(type.cstr(), 0);
    acc->location->type = GEN_URI;
    acc->location->d.ia5 = M_ASN1_IA5STRING_new();
    unsigned char *cident = reinterpret_cast<unsigned char *>(identifier.edit());
    ASN1_STRING_set(acc->location->d.ia5, cident, identifier.len());
}


void WvX509Mgr::set_aia(WvStringList &ca_urls,
                        WvStringList &responders)
{
    AUTHORITY_INFO_ACCESS *ainfo = sk_ACCESS_DESCRIPTION_new_null();

    WvStringList::Iter i(ca_urls);
    for (i.rewind(); i.next();)
        add_aia("caIssuers", i(), ainfo);

    WvStringList::Iter j(responders);
    for (j.rewind(); j.next();)
        add_aia("OCSP", j(), ainfo);

    X509_EXTENSION *ex = X509V3_EXT_i2d(NID_info_access, 0, ainfo);
    X509_add_ext(cert, ex, -1);
    X509_EXTENSION_free(ex);
    sk_ACCESS_DESCRIPTION_pop_free(ainfo, ACCESS_DESCRIPTION_free);
}


WvString WvX509Mgr::get_aia()
{
    return get_extension(NID_info_access);
}


static void parse_stack(WvStringParm ext, WvStringList &list, WvStringParm prefix)
{
    WvStringList stack;
    stack.split(ext, ";\n");
    WvStringList::Iter i(stack);
    for (i.rewind();i.next();)
    {
        WvString stack_entry(*i);
        if (strstr(stack_entry, prefix))
        {
            WvString uri(stack_entry.edit() + prefix.len());
            list.append(uri);  
        }
    }
}


void WvX509Mgr::get_ocsp(WvStringList &responders)
{
    parse_stack(get_aia(), responders, "OCSP - URI:");
}


void WvX509Mgr::get_ca_urls(WvStringList &urls)
{
    parse_stack(get_aia(), urls, "CA Issuers - URI:");
}


void WvX509Mgr::get_crl_urls(WvStringList &urls)
{
    parse_stack(get_crl_dp(), urls, "URI:");
}


void WvX509Mgr::set_crl_urls(WvStringList &urls)
{
    STACK_OF(DIST_POINT) *crldp = sk_DIST_POINT_new_null();
    WvStringList::Iter i(urls);
    for (i.rewind(); i.next();)
    {
        DIST_POINT *point = DIST_POINT_new();
        sk_DIST_POINT_push(crldp, point);

        GENERAL_NAMES *uris = GENERAL_NAMES_new();
        GENERAL_NAME *uri = GENERAL_NAME_new();
        uri->type = GEN_URI;
        uri->d.ia5 = M_ASN1_IA5STRING_new();
        unsigned char *cident = reinterpret_cast<unsigned char *>(i().edit());    
        ASN1_STRING_set(uri->d.ia5, cident, i().len());
        sk_GENERAL_NAME_push(uris, uri);

        point->distpoint = DIST_POINT_NAME_new();
        point->distpoint->name.fullname = uris;
        point->distpoint->type = 0;
    }

    X509_EXTENSION *ex = X509V3_EXT_i2d(NID_crl_distribution_points, 0, crldp);
    X509_add_ext(cert, ex, -1);
    X509_EXTENSION_free(ex);
    sk_DIST_POINT_pop_free(crldp, DIST_POINT_free);
}


bool WvX509Mgr::get_policies(WvStringList &policy_oids)
{
    assert(cert);
    int critical;

    CERTIFICATEPOLICIES * policies = static_cast<CERTIFICATEPOLICIES *>(
        X509_get_ext_d2i(cert, NID_certificate_policies, &critical, NULL));
    if (policies)
    {
        for (int i = 0; i < sk_POLICYINFO_num(policies); i++)
        {
            POLICYINFO * policy = sk_POLICYINFO_value(policies, i);
            const int POLICYID_MAXLEN = 80;

            char policyid[POLICYID_MAXLEN];
            OBJ_obj2txt(policyid, POLICYID_MAXLEN, policy->policyid, 
                        true); // don't substitute human-readable names
            policy_oids.append(policyid);
        }

        sk_POLICYINFO_pop_free(policies, POLICYINFO_free);
        return true;
    }

    return false;
}


void WvX509Mgr::set_policies(WvStringList &policy_oids)
{
    assert(cert);
#if 0
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
	qual->d.cpsouri = M_ASN1_IA5STRING_new();
	ASN1_STRING_set(qual->d.cpsuri, url.edit(), url.len());
	sk_POLICYQUALINFO_push(pol->qualifiers, qual);
    }
    sk_POLICYINFO_push(sk_pinfo, pol);
    X509_EXTENSION *ex = X509V3_EXT_i2d(NID_certificate_policies, 0, 
					sk_pinfo);
    X509_add_ext(cert, ex, -1);
    X509_EXTENSION_free(ex);
    sk_POLICYINFO_free(sk_pinfo);
#endif
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
			char *s = method->i2s(method, ext_data); 
                        retval = s;
                        OPENSSL_free(s);
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


void WvX509Mgr::set_rsakey(WvRSAKey *_rsa)
{   
    WVDELETE(rsa); 
    rsa = _rsa; 
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
	//printf("Looks like:\n%s\n", encode(WvX509Mgr::CertPEM).cstr());
    }
    else if (!X509_check_ca(cert))
    {
        debug("This certificate is not a CA, and is thus not allowed to sign "
              "certificates!\n");
        return false;
    }
    else if (!((cert->ex_flags & EXFLAG_KUSAGE) && 
	  (cert->ex_kusage & KU_KEY_CERT_SIGN)))
    {
	debug("This Certificate is not allowed to sign certificates!\n");
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


bool WvX509Mgr::signcrl(WvCRL *crl)
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
    WvStringList aki_list;
    parse_stack(get_extension(NID_authority_key_identifier), aki_list, "keyid:");
    if (aki_list.count())
        return aki_list.popstr();

    return WvString::null;
}


void WvX509Mgr::set_ski()
{
    ASN1_OCTET_STRING *oct = M_ASN1_OCTET_STRING_new();
    ASN1_BIT_STRING *pk = cert->cert_info->key->public_key;
    unsigned char pkey_dig[EVP_MAX_MD_SIZE];
    unsigned int diglen;

    EVP_Digest(pk->data, pk->length, pkey_dig, &diglen, EVP_sha1(), NULL);

    M_ASN1_OCTET_STRING_set(oct, pkey_dig, diglen);
    X509_EXTENSION *ext = X509V3_EXT_i2d(NID_subject_key_identifier, 0, 
					oct);
    X509_add_ext(cert, ext, -1);
    X509_EXTENSION_free(ext);
    M_ASN1_OCTET_STRING_free(oct);
}


void WvX509Mgr::set_aki(WvX509Mgr &cacert)
{

    // can't set a meaningful AKI for subordinate certification without the 
    // parent having an SKI
    ASN1_OCTET_STRING *ikeyid = NULL;
    X509_EXTENSION *ext;
    int i = X509_get_ext_by_NID(cacert.get_cert(), NID_subject_key_identifier, -1);
    if ((i >= 0) && (ext = X509_get_ext(cacert.get_cert(), i)))
        ikeyid = static_cast<ASN1_OCTET_STRING *>(X509V3_EXT_d2i(ext));

    if (!ikeyid)
        return;

    AUTHORITY_KEYID *akeyid = AUTHORITY_KEYID_new();
    akeyid->issuer = NULL;
    akeyid->serial = NULL;
    akeyid->keyid = ikeyid;
    ext = X509V3_EXT_i2d(NID_authority_key_identifier, 0, akeyid);
    X509_add_ext(cert, ext, -1);
    X509_EXTENSION_free(ext); 
    AUTHORITY_KEYID_free(akeyid);
    //M_ASN1_OCTET_STRING_free(ikeyid);
}
