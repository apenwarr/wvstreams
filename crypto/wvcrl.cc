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

WvCRLMgr::WvCRLMgr(X509_CRL *_crl)
    : debug("X509_CRL", WvLog::Debug5), cacert(NULL), certcount(0), 
      issuer(WvString::null)
{
    err.seterr("Not Initialized yet!");
    if (_crl)
    {
	crl = _crl;
	setupcrl();
	err.noerr();
	
	// Do something about CA Cert.
    }
    else
    {
	debug("Creating new CRL\n");
	if ((crl = X509_CRL_new()) == NULL)
	{
	    err.seterr("Error creating new CRL object");
	    return;
	}
    }
}


WvCRLMgr::~WvCRLMgr()
{
    if (crl)
	X509_CRL_free(crl);
}
    

WvString WvCRLMgr::hexify()
{
    return WvString::null;
}


WvCRLMgr::Valid  WvCRLMgr::validate(WvX509Mgr *cert)
{
    assert(cacert);
    
    if (!cert)
	return ERROR;
    
    if (!(cert->get_issuer() == cacert->get_subject()))
	return NOT_THIS_CA;
    
    if (!(signedbyCA(cert)))
	return NO_VALID_SIGNATURE;
    
    if (isrevoked(cert))
	return REVOKED;
    
    if (X509_cmp_current_time(cert->get_notvalid_before()) > 0)
	return BEFORE_VALID;

    if (X509_cmp_current_time(cert->get_notvalid_after()) < 0)
	return AFTER_VALID;
    
    return VALID;
}


bool WvCRLMgr::signedbyCAindir(WvStringParm certdir)
{
    return false;
}
   

bool WvCRLMgr::signedbyCAinfile(WvStringParm certfile)
{
    return false;
}


bool WvCRLMgr::signedbyCA(WvX509Mgr *cert)
{
    assert(cacert);
    return false;
}


void WvCRLMgr::setca(WvX509Mgr *_cacert)
{
    assert(_cacert);
    cacert = _cacert;
    issuer = cacert->get_issuer();
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
	BIO_write(bufbio, output.get(output.used()), output.used());
	crl = d2i_X509_CRL_bio(bufbio, NULL);
	break;
    case TEXT:
	debug("Sorry, can't decode TEXT format... try PEM or DER instead\n");
	break;
    default:
	err.seterr("Unknown mode!\n");
    }
    setupcrl();
    BIO_free(bufbio);
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
		    return true;
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
    return false;
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


void WvCRLMgr::setupcrl()
{
    char *name = X509_NAME_oneline(X509_CRL_get_issuer(crl), 0, 0);
    issuer = name;
    OPENSSL_free(name);
    STACK_OF(X509_REVOKED) *rev;
    rev = X509_CRL_get_REVOKED(crl);
    certcount = sk_X509_REVOKED_num(rev);
}

