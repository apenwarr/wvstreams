#include "wvbase64.h"
#include "wvsslhacks.h"
#include "wvx509mgr.h"
#include "wvautoconf.h"

#include <openssl/pem.h>
#include <openssl/x509v3.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/sha.h>
#include <openssl/pkcs12.h>


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


WvX509Mgr::WvX509Mgr()
    : WvX509(), 
      debug("X509 Manager", WvLog::Debug5)
{
    rsa = NULL;
}


WvX509Mgr::WvX509Mgr(const WvX509Mgr &x)
    : WvX509(x), 
      debug("X509 Manager", WvLog::Debug5)
{
    rsa = NULL;
    set_rsa(x.rsa);
}


WvX509Mgr::WvX509Mgr(WvStringParm _dname, WvRSAKey *_rsa, bool ca)
    : WvX509(),
      debug("X509 Manager", WvLog::Debug5)
{
    debug("Creating new certificate+key pair for %s.\n", _dname);
    rsa = _rsa;

    if (!!_dname)
    {
        create_selfissued(_dname, ca);
        debug("Ok - Parameters set... now signing certificate.\n");
        signcert(*this);
    }
    else
	debug("Sorry, can't create an anonymous certificate.");
}


WvX509Mgr::WvX509Mgr(WvStringParm _dname, int bits, bool ca)
    : WvX509(), 
      debug("X509 Manager", WvLog::Debug5)
{
    debug("Creating new certificate+key pair for %s.\n", _dname);
    rsa = NULL;
    
    if (!!_dname)
    {
	rsa = new WvRSAKey(bits);
        create_selfissued(_dname, ca);
        debug("Ok - Parameters set... now signing certificate.\n");
        signcert(*this);
    }
    else
	debug("Sorry, can't create an anonymous certificate.");
}


void WvX509Mgr::create_selfissued(WvStringParm dname, bool is_ca)
{
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
	return;

    if ((cert = X509_new()) == NULL)
	return;

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
    
    set_pubkey(*rsa);
		
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
	set_extension(NID_netscape_cert_type,
		      "SSL CA, S/MIME CA, Object Signing CA");
#if 0
        // uncomment this to allow certificate to be used as
        // an OCSP signer (seems too obscure to enable by default
        // right now).
        set_ext_key_usage("OCSP Signing");
#endif
//	debug("Setting Constraints.\n");
//	set_constraints("requireExplicitPolicy");
    }
    else
    {
	debug("Setting Key Usage with normal server parameters\n");
	set_nsserver(dname);
	set_key_usage("critical, digitalSignature, keyEncipherment, "
		      "keyAgreement");
	set_extension(NID_basic_constraints, "CA:FALSE");
	set_ext_key_usage("TLS Web Server Authentication,"
			  "TLS Web Client Authentication");
    }
    
    // we do not actually sign the certificate here: that must be done by the 
    // user (WvX509Mgr most likely)
    
    debug("Certificate for %s created\n", dname);
}


WvX509Mgr::~WvX509Mgr()
{
    debug("Deleting.\n");
    WVDELETE(rsa);
}


bool WvX509Mgr::isok() const
{
    return WvX509::isok() && rsa && rsa->isok() && test();
}


bool WvX509Mgr::operator! () const
{
    return !isok();
}


WvString WvX509Mgr::errstr() const
{
    if (!WvX509::isok())
        return WvX509::errstr();
    
    if (!rsa)
        return "No RSA key set.";
    else if (!rsa->isok())
        return "RSA key not valid.";
    else if (!test())
        return "RSA key and certificate do not match.";

    return WvString::empty;
}


bool WvX509Mgr::bind_ssl(SSL_CTX *ctx)
{
    if (SSL_CTX_use_certificate(ctx, get_cert()) <= 0)
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


bool WvX509Mgr::test() const
{
    if (!cert)
    {
        debug("No X509 certificate: test fails.\n");
        return false;
    }
   
    if (rsa)
    {
        EVP_PKEY *pk = EVP_PKEY_new();
        assert(pk);

	if (!EVP_PKEY_set1_RSA(pk, rsa->rsa))
    	{
            debug("Error setting RSA keys: test fails.\n");
            EVP_PKEY_free(pk);
            return false;
    	}

        bool bad = false;
        int verify_return = X509_verify(cert, pk);
        
        if (verify_return != 1) // only '1' means okay
        {
            // However let's double check:
            WvString rsapub = rsa->encode(WvRSAKey::RsaPubPEM);
            WvRSAKey *temprsa = get_rsa_pub();
            WvString certpub = temprsa->encode(WvRSAKey::RsaPubPEM);
            delete temprsa;
		// debug("rsapub:\n%s\n", rsapub);
            // debug("certpub:\n%s\n", certpub);
		if (certpub == rsapub)
		    ; // do nothing, since OpenSSL is lying
		else
		{
		    // I guess that it really did fail.
		    debug("Certificate test failed: %s\n", wvssl_errstr());
		    bad = true;
		}
        }

        EVP_PKEY_free(pk);
        return !bad;
    }
    
    return false;    
}


WvString WvX509Mgr::signreq(WvStringParm pkcs10req) const
{
    debug("Signing a certificate request with: %s\n", get_subject());
    if (!isok())
    {
        debug(WvLog::Warning, "Asked to sign certificate request, but not ok! "
              "Aborting.\n");
        return false;
    }

    // Break this next part out into a de-pemify section, since that is what
    // this part up until the FIXME: is about.
    WvString pkcs10(pkcs10req);
    
    BIO *membuf = BIO_new(BIO_s_mem());
    BIO_write(membuf, pkcs10req, pkcs10req.len());

    X509_REQ *certreq = PEM_read_bio_X509_REQ(membuf, NULL, NULL, NULL);
    BIO_free_all(membuf);

    if (certreq)
    {
	WvX509 newcert(X509_new());

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
	newcert.set_issuer(*this); 
	
	X509_EXTENSION *ex = NULL;
	// Set the RFC2459-mandated keyUsage field to critical, and restrict
	// the usage of this cert to digital signature and key encipherment.
	newcert.set_key_usage("critical, digitalSignature, keyEncipherment");
    
	// This could cause Netscape to barf because if we set
	// basicConstraints to critical, we break RFC2459 compliance. Why
	// they chose to enforce that bit, and not the rest is beyond me...
	// but oh well...
	ex = X509V3_EXT_conf_nid(NULL, NULL, NID_basic_constraints,
				 (char*)"CA:FALSE");
	
	X509_add_ext(newcert.get_cert(), ex, -1);
	X509_EXTENSION_free(ex);

	newcert.set_ext_key_usage("critical, TLS Web Client Authentication");

	signcert(newcert);
	
	X509_REQ_free(certreq);
	return WvString(newcert.encode(WvX509::CertPEM));
    }
    else
    {
	debug("Can't decode Certificate Request\n");
	return WvString::null;
    }
}


bool WvX509Mgr::signcert(WvX509 &unsignedcert) const
{
    if (!isok())
    {
        debug(WvLog::Warning, "Asked to sign certificate, but not ok! "
              "Aborting.\n");
        return false;
    }

    if (cert == unsignedcert.cert)
    {
	debug("Self Signing!\n");
    }
#ifdef HAVE_OPENSSL_POLICY_MAPPING
    else if (!X509_check_ca(cert))
    {
        debug("This certificate is not a CA, and is thus not allowed to sign "
              "certificates!\n");
        return false;
    }
#endif
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
	X509_sign(unsignedcert.get_cert(), certkey, EVP_sha1());
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


bool WvX509Mgr::signcrl(WvCRL &crl) const
{
    if (!isok() || !crl.isok())
    {
        debug(WvLog::Warning, "Asked to sign CRL, but certificate or CRL (or "
              "both) not ok! Aborting.\n");
        return false;
    }
#ifdef HAVE_OPENSSL_POLICY_MAPPING
    else if (!X509_check_ca(cert))
    {
        debug("This certificate is not a CA, and is thus not allowed to sign "
              "CRLs!\n");
        return false;
    }
    else if (!((cert->ex_flags & EXFLAG_KUSAGE) && 
	  (cert->ex_kusage & KU_CRL_SIGN)))
    {
	debug("Certificate not allowed to sign CRLs! (%s %s)\n", 
              (cert->ex_flags & EXFLAG_KUSAGE),
	      (cert->ex_kusage & KU_CRL_SIGN));
	return false;
    }
#endif
    
    EVP_PKEY *certkey = EVP_PKEY_new();
    bool cakeyok = EVP_PKEY_set1_RSA(certkey, rsa->rsa);
    if (cakeyok)
    {
	ASN1_TIME *tmptm = ASN1_TIME_new();
	// Set the LastUpdate time to now.
	X509_gmtime_adj(tmptm, 0);
	X509_CRL_set_lastUpdate(crl.getcrl(), tmptm);
	// CRL's are valid for 30 days
	X509_gmtime_adj(tmptm, (long)60*60*24*30);
	X509_CRL_set_nextUpdate(crl.getcrl(), tmptm);
	ASN1_TIME_free(tmptm);
	
	// OK - now sign it...
	X509_CRL_sign(crl.getcrl(), certkey, EVP_sha1());
    }
    else
    {
	debug(WvLog::Warning, "Asked to sign CRL, but no RSA key associated "
              "with certificate. Aborting.\n");
	EVP_PKEY_free(certkey);
	return false;
    }
    EVP_PKEY_free(certkey);

    return true;
}


WvString WvX509Mgr::sign(WvStringParm data) const
{
    WvDynBuf buf;
    buf.putstr(data);
    return sign(buf);
}


WvString WvX509Mgr::sign(WvBuf &data) const
{
    assert(rsa);

    EVP_MD_CTX sig_ctx;
    unsigned char sig_buf[4096];
    
    EVP_PKEY *pk = EVP_PKEY_new();
    assert(pk); // OOM 
    
    if (!EVP_PKEY_set1_RSA(pk, rsa->rsa))
    {
	debug("Error setting RSA keys.\n");
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
	debug("Error while signing.\n");
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


bool WvX509Mgr::write_p12(WvStringParm _fname, WvStringParm _pkcs12pass) const
{
    debug("Dumping RSA Key and X509 Cert to PKCS12 structure.\n");

    AutoClose fp = fopen(_fname, "wb");

    if (!fp)
    {
        debug(WvLog::Warning, "Unable to open file. Error: %s\n",
	      strerror(errno));
        return false;
    }

    if (!!_pkcs12pass)
    {
	if (rsa && cert)
	{
	    EVP_PKEY *pk = EVP_PKEY_new();
            assert(pk); // OOM

	    if (!EVP_PKEY_set1_RSA(pk, rsa->rsa))
	    {
		debug("Error setting RSA keys.\n");
		EVP_PKEY_free(pk);
		return false;
	    }
	    else
	    {
                WvString pkcs12pass(_pkcs12pass);
		PKCS12 *pkg
		    = PKCS12_create(pkcs12pass.edit(), (char*)"foo", pk, 
				    cert, NULL, 0, 0, 0, 
				    0, 0);
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
	    debug(WvLog::Warning,
		  "The RSA key or the certificate is not present.");
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
    debug("Reading Certificate and Private Key from PKCS12 file: %s\n",
	  _fname);

    if (rsa)
        WVDELETE(rsa);

    AutoClose fp = fopen(_fname, "r");

    if (!fp)
    {
        debug("Unable to open file '%s'!\n", _fname);
        return;
    }

    if (!!_pkcs12pass)
    {
	PKCS12 *pkg = d2i_PKCS12_fp(fp, NULL);
	if (pkg)
	{
	    EVP_PKEY *pk = NULL;
	    
	    // Parse out the bits out the PKCS12 package.
            X509 *x;
	    PKCS12_parse(pkg, _pkcs12pass, &pk, &x, NULL);
	    PKCS12_free(pkg);
            if (!pk || !x)
            {
                debug("Could not decode pkcs12 file.\n");
                EVP_PKEY_free(pk);
                return;
            }

            cert = x;

	    // Now, cert should be OK, let's try and set up the RSA stuff
	    // since we've essentially got a PKEY, and not a WvRSAKey
	    // We need to create a new WvRSAKey from the PKEY...
	    rsa = new WvRSAKey(EVP_PKEY_get1_RSA(pk), true);
            EVP_PKEY_free(pk);

	    // Now that we have both, check to make sure that they match
	    if (!test())
	    {
		debug("Could not fill in RSA and certificate with matching "
                      "values! Expect problems.\n");
		return;
	    }
	}
	else
	{
	    debug("Read in of PKCS12 file '%s' failed", _fname);
	    return;
	}
    }
    else
    {
        debug("No password specified for PKCS12 file\n");
        return;
    }
}


WvString WvX509Mgr::encode(const WvRSAKey::DumpMode mode) const
{
    if (rsa)
        return rsa->encode(mode);
    return "";
}


WvString WvX509Mgr::encode(const WvX509::DumpMode mode) const
{
    return WvX509::encode(mode);
}


void WvX509Mgr::encode(const WvRSAKey::DumpMode mode, WvBuf &buf) const
{
    if (rsa)
        rsa->encode(mode, buf);
}


void WvX509Mgr::encode(const WvX509::DumpMode mode, WvBuf &buf) const
{
    WvX509::encode(mode, buf);
}


void WvX509Mgr::decode(const WvRSAKey::DumpMode mode, WvStringParm encoded)
{
    if (rsa)
        rsa->decode(mode, encoded);
    else
    {
        rsa = new WvRSAKey();
        rsa->decode(mode, encoded);
    }
}


void WvX509Mgr::decode(const WvX509::DumpMode mode, WvStringParm encoded)
{
    WvX509::decode(mode, encoded);
}


void WvX509Mgr::decode(const WvRSAKey::DumpMode mode, WvBuf &encoded)
{
    if (rsa)
        rsa->decode(mode, encoded);
    else
    {
        rsa = new WvRSAKey();
        rsa->decode(mode, encoded);
    }
}


void WvX509Mgr::decode(const WvX509::DumpMode mode, WvBuf &encoded)
{
    WvX509::decode(mode, encoded);
}
