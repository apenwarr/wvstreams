/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2005 Net Integration Technologies, Inc.
 *
 * X.509v3 CRL management classe.
 */

#include <openssl/x509v3.h>
#include <openssl/pem.h>

#include "wvcrl.h"
#include "wvx509.h"
#include "wvbase64.h"

WvCRL::WvCRL()
    : debug("X509_CRL", WvLog::Debug5), 
      issuer(WvString::null)
{
    crl = NULL;
}


WvCRL::~WvCRL()
{
    debug("Deleting.\n");
    if (crl)
	X509_CRL_free(crl);
}
    

bool WvCRL::signedbyca(WvX509Mgr *cacert)
{
    EVP_PKEY *pkey = X509_get_pubkey(cacert->cert);
    int result = X509_CRL_verify(crl, pkey);    
    EVP_PKEY_free(pkey);
    if (result < 0)
    {
        debug("There was an error determining whether or not we were signed by "
              "CA '%s'\n", cacert->get_subject());
        return false;
    }
    bool issigned = (result > 0);

    debug("CRL was%s signed by CA %s\n", issigned ? "" : " NOT", 
          cacert->get_subject());

    return issigned;
}


bool WvCRL::issuedbyca(WvX509Mgr *cacert)
{
    assert(crl);

    WvString name = get_issuer();
    bool issued = (cacert->get_subject() == name);
    if (issued)
        debug("CRL issuer '%s' matches subject '%s' of cert. We can say "
              "that it appears to be issued by this CA.\n",
              name, cacert->get_subject());
    else
        debug("CRL issuer '%s' doesn't match subject '%s' of cert. Doesn't "
              "appear to be issued by this CA.\n", name, 
              cacert->get_subject());

    return issued;
}


bool WvCRL::expired()
{
    if (X509_cmp_current_time(X509_CRL_get_nextUpdate(crl)) < 0)
    {
        debug("CRL appears to be expired.\n");
	return true;
    }

    debug("CRL appears not to be expired.\n");
    return false;
}


bool WvCRL::has_critical_extensions()
{
    int critical = X509_CRL_get_ext_by_critical(crl, 1, 0);
    return (critical > 0);
}


WvString WvCRL::get_aki()
{
    assert(crl);

    AUTHORITY_KEYID *aki = NULL;
    int i;

    printf("Die\n");
    aki = static_cast<AUTHORITY_KEYID *>(
        X509_CRL_get_ext_d2i(crl, NID_authority_key_identifier, 
                             &i, NULL));
    printf("Dead?\n");
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


WvString WvCRL::get_issuer()
{ 
    assert(crl);

    char *name = X509_NAME_oneline(X509_CRL_get_issuer(crl), 0, 0);
    WvString retval(name);
    OPENSSL_free(name);
    return retval;
}


WvString WvCRL::encode(const DumpMode mode)
{
    BIO *bufbio = BIO_new(BIO_s_mem());    
    BUF_MEM *bm;
    switch (mode)
    {
    case PEM:
	debug("Dumping CRL in PEM format.\n");
	PEM_write_bio_X509_CRL(bufbio, crl);
	break;
    case DER:
	debug("Dumping CRL in DER format.\n");
	i2d_X509_CRL_bio(bufbio, crl);
	break;
    case TEXT:
	debug("Dumping CRL in human readable format.\n");
	X509_CRL_print(bufbio, crl);
	break;
    default:
	err.seterr("Unknown mode!\n");
	return WvString::null;
    }
    
    WvDynBuf retval;
    BIO_get_mem_ptr(bufbio, &bm);
    retval.put(bm->data, bm->length);
    BIO_free(bufbio);
    if (mode == DER)
    {
        WvBase64Encoder enc;
        WvString output;
        enc.flushbufstr(retval, output, true);
        return output;
    }
    else
        return retval.getstr();
}


void WvCRL::decode(const DumpMode mode, WvStringParm encoded)
{
    WvDynBuf buf;
    buf.putstr(encoded);
    decode(mode, buf);
}


void WvCRL::decode(const DumpMode mode, WvBuf &buf)
{
    if (crl)
    {
	debug("Replacing already existant CRL\n");
	X509_CRL_free(crl);
	crl = NULL;
    }

    BIO *bufbio = BIO_new(BIO_s_mem());

    WvBase64Decoder dec;
    WvDynBuf output;

    switch (mode)
    {
    case PEM:
	debug("Decoding CRL from PEM format.\n");
	BIO_write(bufbio, buf.get(buf.used()), buf.used());
	crl = PEM_read_bio_X509_CRL(bufbio, NULL, NULL, NULL);
        assert(crl);
        if (!crl)
            err.seterr("Couldn't decode CRL from PEM format.\n");
	break;
    case DER:
        debug("Decoding CRL from DER format.\n");
	BIO_write(bufbio, buf.get(buf.used()), buf.used());
        crl = d2i_X509_CRL_bio(bufbio, NULL);
        if (!crl)
            err.seterr("Couldn't decode CRL from DER format.\n");
        break;
    case DER64:
	debug("Decoding CRL from DER format encoded in base64.\n");
        dec.encode(buf, output, true, true);
	BIO_write(bufbio, output.get(output.used()), output.used());
	crl = d2i_X509_CRL_bio(bufbio, NULL);
        if (!crl)
            err.seterr("Couldn't decode CRL from DER format encoded in BASE64.\n");
	break;
    default:
	err.seterr("Unknown mode!\n");
    }

    BIO_free(bufbio);
}


void WvCRL::load(const DumpMode mode, WvStringParm fname)
{
    if (crl)
    {
	debug("Replacing already existant CRL\n");
	X509_CRL_free(crl);
	crl = NULL;
    }

    if (mode == DER)
    {
	BIO *bio = BIO_new(BIO_s_file());
        if (BIO_read_filename(bio, fname.cstr()) <= 0)
        {
            debug("Loading non-existent CRL?\n");
            err.seterr(errno);
            BIO_free(bio);
            return;
        }
        if (!(crl = d2i_X509_CRL_bio(bio, &crl)))
        {
            debug(WvLog::Warning, "Tried to load CRL, but was invalid.\n");
        }
        BIO_free(bio);
        return;
    }

    // FIXME: we don't support anything else
    assert(0);
}


bool WvCRL::isrevoked(WvX509Mgr *cert)
{
    if (cert && cert->get_cert())
    {
        debug("Checking to see if certificate with name '%s' and serial "
              "number '%s' is revoked.\n", cert->get_subject(), 
              cert->get_serial());
	return isrevoked(cert->get_serial());
    }
    else
    {
	debug(WvLog::Error, "Given certificate to check revocation status, "
              "but certificate is bad. Declining.\n");
	return true;
    }
}


bool WvCRL::isrevoked(WvStringParm serial_number)
{
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
		debug("CRL does not have revoked list.");
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
    

ASN1_INTEGER *WvCRL::serial_to_int(WvStringParm serial)
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
    else
	return NULL;
}

WvCRL::Valid WvCRL::validate(WvX509Mgr *cacert)
{
    assert(cacert);
    
    if (!cacert)
	return CRLERROR;

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



#if 0
int WvCRL::numcerts()
{
    return certcount;
}


void WvCRL::addcert(WvX509Mgr *cert)
{
    if (cert && cert->isok())
    {
	ASN1_INTEGER *serial = serial_to_int(cert->get_serial());
	X509_REVOKED *revoked = X509_REVOKED_new();
	ASN1_GENERALIZEDTIME *now = ASN1_GENERALIZEDTIME_new();
	X509_REVOKED_set_serialNumber(revoked, serial);
	X509_gmtime_adj(now, 0);
	X509_REVOKED_set_revocationDate(revoked, now);
	// FIXME: We don't deal with the reason here...
	X509_CRL_add0_revoked(crl, revoked);
	ASN1_GENERALIZEDTIME_free(now);
	ASN1_INTEGER_free(serial);
	certcount++;
    }
    else
    {
	debug(WvLog::Warning, "Tried to add a certificate to the CRL, but "
              "certificate is either bad or broken.\n");
    }
}


void WvCRL::setupcrl()
{
    char *name = X509_NAME_oneline(X509_CRL_get_issuer(crl), 0, 0);
    issuer = name;
    OPENSSL_free(name);
    STACK_OF(X509_REVOKED) *rev;
    rev = X509_CRL_get_REVOKED(crl);
    certcount = sk_X509_REVOKED_num(rev);
}
#endif
