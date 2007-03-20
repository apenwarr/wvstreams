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

WvCRLMgr::WvCRLMgr()
    : debug("X509_CRL", WvLog::Debug5), 
      issuer(WvString::null)
{
    crl = NULL;
}


WvCRLMgr::~WvCRLMgr()
{
    if (crl)
	X509_CRL_free(crl);
}
    

bool WvCRLMgr::signedbyca(WvX509Mgr *cacert)
{
    // FIXME: implement
    return false;
}


WvString WvCRLMgr::encode(const DumpMode mode)
{
    BIO *bufbio = BIO_new(BIO_s_mem());    
    BUF_MEM *bm;
    switch (mode)
    {
    case PEM:
	debug("Dumping CRL in PEM format:\n");
	PEM_write_bio_X509_CRL(bufbio, crl);
	break;
    case DER:
	debug("Dumping CRL in DER format:\n");
	i2d_X509_CRL_bio(bufbio, crl);
	break;
    case TEXT:
	debug("Dumping CRL in human readable format:\n");
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


void WvCRLMgr::decode(const DumpMode mode, WvStringParm PemEncoded)
{
    BIO *bufbio = BIO_new(BIO_s_mem());    
    WvBase64Decoder dec;
    WvDynBuf output;

    if (crl)
    {
	debug("Replacing already existant CRL\n");
	X509_CRL_free(crl);
	crl = NULL;
    }
    
    size_t output_size;
    switch (mode)
    {
    case PEM:
	debug("Decoding CRL from PEM format:\n");
	BIO_write(bufbio, PemEncoded.cstr(), PemEncoded.len());
	crl = PEM_read_bio_X509_CRL(bufbio, NULL, NULL, NULL);
	break;
    case DER:
	debug("Decoding CRL from DER format:\n");
	dec.flushstrbuf(PemEncoded, output, true);
	output_size = output.used();
	BIO_write(bufbio, output.get(output_size), output_size);
	crl = d2i_X509_CRL_bio(bufbio, NULL);
	break;
    case TEXT:
	debug("Sorry, can't decode TEXT format... try PEM or DER instead\n");
	break;
    default:
	err.seterr("Unknown mode!\n");
    }
    //setupcrl();
    BIO_free(bufbio);
}


void WvCRLMgr::load(const DumpMode mode, WvStringParm fname)
{
    if (crl)
    {
	debug("Replacing already existant CRL\n");
	X509_CRL_free(crl);
	crl = NULL;
    }

    if (mode == ASN1)
    {
	BIO *bio = BIO_new(BIO_s_file());
        if (BIO_read_filename(bio, fname.cstr()) <= 0)
        {
            debug("Loading non-existent CRL?\n");
            err.seterr(errno);
            return;
        }
        crl = d2i_X509_CRL_bio(bio, NULL);
        BIO_free(bio);
        return;
    }

    // FIXME: we don't support anything else
    assert(0);
}


WvString WvCRLMgr::get_issuer()
{
    if (crl)
	return issuer;

    return WvString::null;
}


bool WvCRLMgr::isrevoked(WvX509Mgr *cert)
{
    if (cert && cert->isok())
	return isrevoked(cert->get_serial());
    else
    {
	debug(WvLog::Critical,"Given bad certificate... declining\n");
	return true;
    }
}


bool WvCRLMgr::isrevoked(WvStringParm serial_number)
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
	    }
	    else
	    {
		ASN1_INTEGER_free(serial);
		debug("No CRL Revoked list? I guess %s isn't in it!\n", 
		      serial_number);
	    }
	    
	}
	else
	    debug("Can't convert serial number...odd!\n");
    }
    else
    {
	debug("Can't check imaginary serial number!\n");
    }

    debug("Certificate is not revoked (or could not determine whether it "
          "was.\n");
    return false;
}
    

ASN1_INTEGER *WvCRLMgr::serial_to_int(WvStringParm serial)
{
    debug(WvLog::Critical, "Converting: %s\n", serial);
    if (!!serial)
    {
	ASN1_INTEGER *retval = ASN1_INTEGER_new();
	ASN1_INTEGER_set(retval, serial.num());
	return retval;
    }
    else
	return NULL;
}

#if 0
WvCRLMgr::Valid WvCRLMgr::validate(WvX509Mgr *cert)
{
    assert(cacert);
    
    if (!cert)
	return CRLERROR;
    
    if (!(cert->get_issuer() == cacert->get_subject()))
	return NOT_THIS_CA;
    
    if (!(signedbyCA(cert)))
	return NO_VALID_SIGNATURE;
    
    if (isrevoked(cert))
	return REVOKED;
    
    if (X509_cmp_current_time(X509_get_notBefore(cert->get_cert())) > 0)
	return BEFORE_VALID;

    if (X509_cmp_current_time(X509_get_notBefore(cert->get_cert())) < 0)
	return AFTER_VALID;
    
    return VALID;
}

int WvCRLMgr::numcerts()
{
    return certcount;
}


void WvCRLMgr::addcert(WvX509Mgr *cert)
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
	debug("Sorry, can't add a certificate that is either bad or broken\n");
    }
}


void WvCRLMgr::setupcrl()
{
    char *name = X509_NAME_oneline(X509_CRL_get_issuer(crl), 0, 0);
    issuer = name;
    OPENSSL_free(name);
    STACK_OF(X509_REVOKED) *rev;
    rev = X509_CRL_get_REVOKED(crl);
    certcount = sk_X509_REVOKED_num(rev);
}
#endif
