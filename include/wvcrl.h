/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2007 Net Integration Technologies, Inc. and others.
 *
 * X.509v3 CRL management classes.
 */ 
#ifndef __WVCRL_H
#define __WVCRL_H

#include "wverror.h"
#include "wvlog.h"
#include "wvx509.h"

// Structures to make the compiler happy so we don't have to include x509v3.h ;)
struct X509_crl_st;
typedef struct X509_crl_st X509_CRL;
struct ssl_ctx_st;
typedef struct ssl_ctx_st SSL_CTX;
struct asn1_string_st;
typedef struct asn1_string_st ASN1_INTEGER;


/**
 * CRL Class to handle certificate revocation lists and their related
 * functions
 */
class WvCRL
{
public:
    /**
     * Where errors go when they happen
     */
    WvError err;

    /**
     * Type for the @ref encode() and @ref decode() methods:
     * CRLPEM   = PEM Encoded X.509 CRL
     * CRLDER   = DER Encoded X.509 CRL returned in Base64
     * TEXT     = Decoded Human readable format.
     */
    enum DumpMode { PEM = 0, DER, DER64, TEXT };

    /**
     * Initialize a blank CRL Object.
     */
    WvCRL();
    
    /** Destructor */
    virtual ~WvCRL();

    /** Accessor for CRL */
    X509_CRL *getcrl()
    { return crl; }
 
    /**
     * Check the CRL in crl against the CA certificate in cert
     * - returns true if CRL was signed by that CA certificate.
     */
    bool signedbyca(WvX509 &cacert);

    /**
     * Check the issuer name of the CRL in crl against the CA certificate in cert
     * - returns true if the names match.
     */
    bool issuedbyca(WvX509 &cacert);

    /**
     * Checks to see if the CRL is expired (i.e.: the present time is past the
     * nextUpdate extension).
     * - returns true if CRL has expired.
     */
    bool expired();

    /*
     * Checks to see if the CRL has any critical extensions in it.
     * - returns true if the CRL has any critical extensions.
     */
    bool has_critical_extensions();

    /**
     * Type for @ref validate() method:
     * ERROR = there was an error that happened..
     * VALID = the certificate is valid
     * NOT_THIS_CA = the certificate is not signed by this CA
     * NO_VALID_SIGNATURE = the certificate claims to be signed by this CA (Issuer is the same),
     *                      but the signature is invalid.
     */    
    enum Valid { CRLERROR = -1, VALID, NOT_THIS_CA, NO_VALID_SIGNATURE, 
                 EXPIRED, UNHANDLED_CRITICAL_EXTENSIONS };

    /**
     * Checks to see that a CRL is signed and issued by a CA certificate, and
     * that it has not expired.
     * - returns a validity status.
     * Get the Authority key Info
     */
    Valid validate(WvX509 &cacert);

    /**
     * Get the Authority key Info
     */
    WvString get_aki();

    /** 
     * Get the CRL Issuer.
     */
    WvString get_issuer();

    /**
     * Do we have any errors... convenience function..
     */
    bool isok()
    { return err.isok(); }  
    
    /** 
     * Return the information requested by mode as a WvString. 
     */
    WvString encode(const DumpMode mode);

    /**
     * Load the information from the format requested by mode into
     * the class - this overwrites the CRL.
     */
    void decode(const DumpMode mode, WvStringParm PemEncoded);
    void decode(const DumpMode mode, WvBuf &encoded);

    /**
     * Loads a CRL from a file on disk.
     */
    void load(const DumpMode mode, WvStringParm fname);

    /**
     * Is the certificate in cert revoked?
     */
    bool isrevoked(WvX509 &cert);
    bool isrevoked(WvStringParm serial_number);
    
private:
    WvLog debug;

    X509_CRL     *crl;
    WvString     issuer;

    ASN1_INTEGER *serial_to_int(WvStringParm serial);
};

#endif // __WVCRL_H
