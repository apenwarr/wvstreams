/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2005 Net Integration Technologies, Inc.
 *
 * X.509v3 CRL management classes.
 */

#include <openssl/x509v3.h>
#include <openssl/pem.h>

#include "wvcrl.h"
#include "wvx509mgr.h"
#include "wvbase64.h"

static const char * warning_str_get = "Tried to determine %s, but CRL is blank!\n";
#define CHECK_CRL_EXISTS_GET(x, y)                                      \
    if (!crl) {                                                         \
        debug(WvLog::Warning, warning_str_get, x);                      \
        return y;                                                       \
    }

static ASN1_INTEGER * serial_to_int(WvStringParm serial)
{
    if (!!serial)
    {
        BIGNUM *bn = NULL;
        BN_dec2bn(&bn, serial);
        ASN1_INTEGER *retval = ASN1_INTEGER_new();
        retval = BN_to_ASN1_INTEGER(bn, retval);
        BN_free(bn);
	return retval;
    }

    return NULL;
}


WvCRL::WvCRL()
    : debug("X509 CRL", WvLog::Debug5)
{
    crl = NULL;
}


WvCRL::WvCRL(const WvX509Mgr &cacert)
    : debug("X509 CRL", WvLog::Debug5)
{
    assert(crl = X509_CRL_new());
    cacert.signcrl(*this);
}


WvCRL::~WvCRL()
{
    debug("Deleting.\n");
    if (crl)
	X509_CRL_free(crl);
}


bool WvCRL::isok() const
{
    return crl;
}
    

bool WvCRL::signedbyca(const WvX509 &cacert) const
{
    CHECK_CRL_EXISTS_GET("if CRL is signed by CA", false);

    EVP_PKEY *pkey = X509_get_pubkey(cacert.cert);
    int result = X509_CRL_verify(crl, pkey);
    EVP_PKEY_free(pkey);
    if (result < 0)
    {
        debug("There was an error determining whether or not we were signed by "
              "CA '%s'\n", cacert.get_subject());
        return false;
    }
    bool issigned = (result > 0);

    debug("CRL was%s signed by CA %s\n", issigned ? "" : " NOT", 
          cacert.get_subject());

    return issigned;
}


bool WvCRL::issuedbyca(const WvX509 &cacert) const
{
    CHECK_CRL_EXISTS_GET("if CRL is issued by CA", false);

    WvString name = get_issuer();
    bool issued = (cacert.get_subject() == name);
    if (issued)
        debug("CRL issuer '%s' matches subject '%s' of cert. We can say "
              "that it appears to be issued by this CA.\n",
              name, cacert.get_subject());
    else
        debug("CRL issuer '%s' doesn't match subject '%s' of cert. Doesn't "
              "appear to be issued by this CA.\n", name, 
              cacert.get_subject());

    return issued;
}


bool WvCRL::expired() const
{
    CHECK_CRL_EXISTS_GET("if CRL has expired", false);

    if (X509_cmp_current_time(X509_CRL_get_nextUpdate(crl)) < 0)
    {
        debug("CRL appears to be expired.\n");
	return true;
    }

    debug("CRL appears not to be expired.\n");
    return false;
}


bool WvCRL::has_critical_extensions() const
{
    CHECK_CRL_EXISTS_GET("if CRL has critical extensions", false);

    int critical = X509_CRL_get_ext_by_critical(crl, 1, 0);
    return (critical > 0);
}


WvString WvCRL::get_aki() const
{
    CHECK_CRL_EXISTS_GET("CRL's AKI", WvString::null);

    AUTHORITY_KEYID *aki = NULL;
    int i;

    aki = static_cast<AUTHORITY_KEYID *>(
        X509_CRL_get_ext_d2i(crl, NID_authority_key_identifier, 
                             &i, NULL));
    if (aki)
    {
        char *tmp = hex_to_string(aki->keyid->data, aki->keyid->length); 
        WvString str(tmp);
        
        OPENSSL_free(tmp);
        AUTHORITY_KEYID_free(aki);
       
        return str;
    }

    return WvString::null;
}


WvString WvCRL::get_issuer() const
{ 
    CHECK_CRL_EXISTS_GET("CRL's issuer", WvString::null);

    char *name = X509_NAME_oneline(X509_CRL_get_issuer(crl), 0, 0);
    WvString retval(name);
    OPENSSL_free(name);

    return retval;
}


WvString WvCRL::encode(const DumpMode mode) const
{
    WvDynBuf retval;
    encode(mode, retval);

    return retval.getstr();
}


void WvCRL::encode(const DumpMode mode, WvBuf &buf) const
{
    if (mode == CRLFileDER || mode == CRLFilePEM)
        return; // file modes are no ops with encode

    if (!crl)
    {
        debug(WvLog::Warning, "Tried to encode CRL, but CRL is blank!\n");
        return;
    }

    BIO *bufbio = BIO_new(BIO_s_mem());
    BUF_MEM *bm;
    switch (mode)
    {
    case CRLPEM:
	debug("Dumping CRL in PEM format.\n");
	PEM_write_bio_X509_CRL(bufbio, crl);
	break;
    case CRLDER:
	debug("Dumping CRL in DER format.\n");
	i2d_X509_CRL_bio(bufbio, crl);
	break;
    default:
        debug("Tried to dump CRL in unknown format!\n");
        break;
    }
    
    BIO_get_mem_ptr(bufbio, &bm);
    buf.put(bm->data, bm->length);
    BIO_free(bufbio);
}


void WvCRL::decode(const DumpMode mode, WvStringParm str)
{
    if (crl)
    {
	debug("Replacing already existant CRL.\n");
	X509_CRL_free(crl);
	crl = NULL;
    }

    if (mode == CRLFileDER)
    {
        BIO *bio = BIO_new(BIO_s_file());
        
        if (BIO_read_filename(bio, str.cstr()) <= 0)
        {
            debug(WvLog::Warning, "Import CRL from '%s': %s\n", 
                  str, wvssl_errstr());
            BIO_free(bio);
            return;
        }
        
        if (!(crl = d2i_X509_CRL_bio(bio, NULL)))
            debug(WvLog::Warning, "Read CRL from '%s': %s\n",
		  str, wvssl_errstr());
        
        BIO_free(bio);
        return;
    }
    else if (mode == CRLFilePEM)
    {
        FILE * fp = fopen(str, "rb");
        if (!fp)
        {
	    int errnum = errno;
            debug(WvLog::Warning,
		  "Import CRL from '%s': %s\n", 
                  str, strerror(errnum));
            return;
        }

        if (!(crl = PEM_read_X509_CRL(fp, NULL, NULL, NULL)))
            debug(WvLog::Warning, "Can't read CRL from file");
        
        fclose(fp);
        return;
    }

    // we use the buffer decode functions for everything else
    WvDynBuf buf;
    buf.putstr(str);
    decode(mode, buf);
}


void WvCRL::decode(const DumpMode mode, WvBuf &buf)
{
    if (crl)
    {
	debug("Replacing already existant CRL.\n");
	X509_CRL_free(crl);
	crl = NULL;
    }

    if (mode == CRLFileDER || mode == CRLFilePEM)
    {
        decode(mode, buf.getstr());
        return;
    }

    BIO *bufbio = BIO_new(BIO_s_mem());
    BIO_write(bufbio, buf.get(buf.used()), buf.used());

    if (mode == CRLPEM)
    {
	debug("Decoding CRL from PEM format.\n");	
	crl = PEM_read_bio_X509_CRL(bufbio, NULL, NULL, NULL);
    }
    else if (mode == CRLDER)
    {
        debug("Decoding CRL from DER format.\n");
        crl = d2i_X509_CRL_bio(bufbio, NULL);
    }
    else
        debug(WvLog::Warning, "Attempted to decode unknown format.\n");

    if (!crl)
        debug(WvLog::Warning, "Couldn't decode CRL.\n");

    BIO_free(bufbio);
}


bool WvCRL::isrevoked(const WvX509 &cert) const
{
    if (cert.cert)
    {
        debug("Checking to see if certificate with name '%s' and serial "
              "number '%s' is revoked.\n", cert.get_subject(), 
              cert.get_serial());
	return isrevoked(cert.get_serial());
    }
    else
    {
	debug(WvLog::Error, "Given certificate to check revocation status, "
              "but certificate is blank. Declining.\n");
	return true;
    }
}


bool WvCRL::isrevoked(WvStringParm serial_number) const
{
    CHECK_CRL_EXISTS_GET("if certificate is revoked in CRL", false);

    if (!!serial_number)
    {
	ASN1_INTEGER *serial = serial_to_int(serial_number);
	if (serial)
	{
	    X509_REVOKED mayberevoked;
	    mayberevoked.serialNumber = serial;
	    if (crl->crl->revoked)
	    {
		int idx = sk_X509_REVOKED_find(crl->crl->revoked, 
					       &mayberevoked);
		ASN1_INTEGER_free(serial);
		if (idx >= 0)
                {
                    debug("Certificate is revoked.\n");
		    return true;
                }
                else
                {
                    debug("Certificate is not revoked.\n");
		    return false;
                }
	    }
	    else
	    {
		ASN1_INTEGER_free(serial);
		debug("CRL does not have revoked list.\n");
                return false;
	    }
	    
	}
	else
	    debug(WvLog::Warning, "Can't convert serial number to ASN1 format. "
                  "Saying it's not revoked.\n");
    }
    else
	debug(WvLog::Warning, "Serial number for certificate is blank.\n");

    debug("Certificate is not revoked (or could not determine whether it "
          "was).\n");
    return false;
}
    

WvCRL::Valid WvCRL::validate(const WvX509 &cacert) const
{
    if (!issuedbyca(cacert))
        return NOT_THIS_CA;
    
    if (!signedbyca(cacert))
 	return NO_VALID_SIGNATURE;

    if (expired())
        return EXPIRED;

    // neither we or openssl handles any critical extensions yet
    if (has_critical_extensions())
    {
        debug("CRL has unhandled critical extensions.\n");
        return UNHANDLED_CRITICAL_EXTENSIONS;
    }

    return VALID;
}


int WvCRL::numcerts() const
{
    CHECK_CRL_EXISTS_GET("number of certificates in CRL", 0);

    STACK_OF(X509_REVOKED) *rev;
    rev = X509_CRL_get_REVOKED(crl);
    int certcount = sk_X509_REVOKED_num(rev);
    
    if (certcount < 0)
        certcount = 0;

    return certcount;
}


void WvCRL::addcert(const WvX509 &cert)
{
    if (!crl)
    {
        debug(WvLog::Warning, "Tried to add certificate to CRL, but CRL is "
              "blank!\n");
        return;
    }

    if (cert.isok())
    {
	ASN1_INTEGER *serial = serial_to_int(cert.get_serial());
	X509_REVOKED *revoked = X509_REVOKED_new();
	ASN1_GENERALIZEDTIME *now = ASN1_GENERALIZEDTIME_new();
	X509_REVOKED_set_serialNumber(revoked, serial);
	X509_gmtime_adj(now, 0);
	X509_REVOKED_set_revocationDate(revoked, now);
	// FIXME: We don't deal with the reason here...
	X509_CRL_add0_revoked(crl, revoked);
	ASN1_GENERALIZEDTIME_free(now);
	ASN1_INTEGER_free(serial);
    }
    else
    {
	debug(WvLog::Warning, "Tried to add a certificate to the CRL, but "
              "certificate is either bad or broken.\n");
    }
}

