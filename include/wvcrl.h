/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2005 Net Integration Technologies, Inc.
 *
 * X.509v3 CRL management classes.
 */ 
#ifndef __WVCRL_H
#define __WVCRL_H

#include "wvlog.h"
#include "wverror.h"

// Structures to make the compiler happy so we don't have to include x509v3.h ;)
struct X509_crl_st;
typedef struct X509_crl_st X509_CRL;
struct ssl_ctx_st;
typedef struct ssl_ctx_st SSL_CTX;
struct asn1_string_st;
typedef struct asn1_string_st ASN1_INTEGER;

class WvRSAKey;
class WvX509Mgr;

// workaround for the fact that OpenSSL initialization stuff must be called
// only once.
void wvssl_init();
void wvssl_free();
WvString wvssl_errstr();

/**
 * CRL Class to handle certificates and their related
 * functions
 */
class WvCRLMgr
{
public:
    WvError err;
   /**
    * Type for the @ref encode() and @ref decode() methods.
    * CRLPEM   = PEM Encoded X.509 CRL
    * CRLDER   = DER Encoded X.509 CRL returned in Base64
    * TEXT     = Decoded Human readable format.
    */
    enum DumpMode { PEM = 0, DER, TEXT };
    /**
     * Type for @ref validate method
     * YES = this certificate is valid
     * NOT_THIS_CA = certificate is not signed by this CA
     * NO_VALID_SIGNATURE = certificate claims to be signed by this CA (Issuer is the same),
     *                      but the signature is invalid.
     * BEFORE_VALID = certificate has not become valid yet
     * AFTER_VALID = ceriticate is past it's validity period
     * REVOKED = certificate has been revoked (it's serial number is in this CRL)
     */
    
    enum Valid { ERROR = -1, VALID , NOT_THIS_CA, NO_VALID_SIGNATURE, BEFORE_VALID, AFTER_VALID, REVOKED };
    
    /**
     * Initialize a blank CRL Object
     * 
     * This either initializes a completely empty object, or takes
     * a pre-allocated _crl - takes ownership.
     */
    WvCRLMgr(X509_CRL *_crl = NULL);
    
private:
    /** 
     * Placeholder for Copy Constructor: this doesn't exist yet, but it keeps
     * us out of trouble :) 
     */
    WvCRLMgr(const WvCRLMgr &mgr);

public:
    /** Destructor */
    virtual ~WvCRLMgr();
    

    /** Accessor for CRL */
    X509_CRL *getcrl()
    { return crl; }
 

    /**
     * Given the CRL object crl, return a hexified string
     * useful in a WvConf or UniConf file.
     * 
     */
    WvString hexify();

    /**
     * Function to verify the validity of a certificate given by
     * cert. This function checks three things:
     * 1: That the certificate has been issued by the same CA that
     *    has signed this CRL.
     * 2: That the certificate is within it's validity range
     * 3: That the certificate isn't in the CRL.
     */
    Valid validate(WvX509Mgr *cert);

    /**
     * Check the CRL in crl  against the CA certificates in
     * certdir - returns true if crl was signed by one of the CA
     * certificates.
     */
    bool signedbyCAindir(WvStringParm certdir);
   
    /**
     * Check the CRL in crl against the CA certificate in certfile
     * - returns true if crl was signed by that CA certificate. 
     */
   bool signedbyCAinfile(WvStringParm certfile);

   /**
    * Check the CRL in crl against the CA certificate in cacert
    * - returns true if CRL was signed by that CA certificate.
    */
    bool signedbyCA(WvX509Mgr *cert);

    bool isok()
    { return err.isok(); }
    
    /**
     * Set the CA for this certificate...
     */
    void setca(WvX509Mgr *cacert);
    
    
    /** 
     * Return the information requested by mode as a WvString. 
     */
    WvString encode(const DumpMode mode);

    /**
     * Load the information from the format requested by mode into
     * the class - this overwrites the certificate, and possibly the
     * key - and to enable two stage loading (the certificate first, then the
     * key), it DOES NOT call test() - that will be up to the programmer
     */
    void decode(const DumpMode mode, WvStringParm PemEncoded);

    /** 
     * Return the CRL Issuer (usually the CA who signed 
     * the certificate)
     */
    WvString get_issuer();

    /**
     * Is the certificate in cert revoked?
     */
    bool isrevoked(WvX509Mgr *cert);
    bool isrevoked(WvStringParm serial_number);
    
    /**
     * How many certificates in the CRL?
     */
    int numcerts();

    /**
     * Add the certificate in cert to the CRL
     */
    void addcert(WvX509Mgr *cert);

private:
    /** X.509v3 CRL - this is why this class exists */
    WvLog debug;

    X509_CRL     *crl;
    WvX509Mgr    *cacert;
    int          certcount;
    WvString     issuer;

    ASN1_INTEGER *serial_to_int(WvStringParm serial);
    void	 setupcrl();

};

#endif // __WVCRL_H
