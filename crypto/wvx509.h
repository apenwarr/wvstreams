/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * X.509 certificate management classes.
 */ 
#ifndef __WVX509_H
#define __WVX509_H

#include "wvlog.h"

// Structures to make the compiler happy so we don't have to include x509v3.h ;)
struct x509_st;
typedef struct x509_st X509;

class WvRSAKey;


// workaround for the fact that OpenSSL initialization stuff must be called
// only once.
void wvssl_init();
void wvssl_free();
WvString wvssl_errstr();


/**
 * X509 Class to Handle certificates and their related
 * functions
 */
class WvX509Mgr
{
public:
    /**
     * Initialize a blank X509 Object with the certificate *cert
     * (used for client side operations...)
     */
    WvX509Mgr(X509 *_cert = NULL);

    /**
     * Constructor to create a selfsigned certificate for dn dName
     * NOTE: If you already have an RSAKey, then you can shove it
     * in here in the third parameter (i.e.: If you wanted to generate a
     * cert for an existing TunnelVision connection).
     *
     * Also: For SSL Servers:
     * the dName MUST be in the form: cn=FQDN,o=foo,c=CA
     * (actually, any part after the cn=FQDN is up to you... dc= works as well..)
     *
     * But the important bit is to use the Fully Qualified Domain Name in 
     * the cn= part - otherwise Web Clients get confused...(I imaging other
     * server clients will get equally confused, but I haven't checked).
     * I don't check for this, since other kinds of certificates are perfectly
     * valid without this... If you want to generate invalid certs, that's up
     * to you.
     */
    WvX509Mgr(WvString dName, int bits, WvRSAKey *_rsa = NULL);

    /**
     * Destructor
     */
    virtual ~WvX509Mgr();

    /**
     * Certificate - this is why this class exists
     */
    X509     *cert;

    /**
     * The Public and Private RSA keypair associated with this certificate
     * Make sure that you save this somewhere!!! If you don't, then you won't
     * really be able to use the certificate for anything...
     */
    WvRSAKey *rsa;
    
    /**
     * A hexified encoding of cert for use in getting it in and out of WvConf
     * objects. I don't provide a similar entry for that for keypair, because
     * you can always call rsa->private_str() and rsa->public_str()
     * for that information.
     */
    WvString enccert;

    /**
     * Given the Distinguished Name dName and the number of bits for the
     * Private key in keysize, return a Self Signed Certificate, and the RSA
     * Private/Public Keypair in rsa
     */
    void createSScert(WvString dName, int keysize);

    /**
     * Create a certificate request (PKCS#10) using this function.. this 
     * request is what you would send off to Verisign, or Entrust.net (or any
     * other CA), to get your real certificate. It leaves the RSA key pair
     * in rsa, where you MUST save it for the certificate to be AT ALL
     * valid when you get it back. Returns a PEM Encoded PKCS#10 certificate
     * request.
     */    
    WvString createcertreq(WvString dName, int keysize);
    
    /**
     * test to make sure that a certificate and a keypair go together.
     * called internally by decodecert() although you can call it if 
     * you want to load a certificate yourself
     */
    bool testcert();

    /**
     * Given a hexified encodedcert, fill the cert member
     * NOTE: ALWAYS load your RSA Keys before calling this!
     */
    void decodecert(WvString encodedcert);
    
    /**
     * Given the X509 certificate object cert, return a hexified string
     * (in enccert) - Suitable for inclusion in a WvConf object ;)
     */
    void encodecert();

    /**
     * Function to verify the validity of a certificate that has been
     * placed in cert. Currently, this only outputs some information about
     * the certificate but eventually, it should be used to verify that the
     * certificate is valid (has not expired, and was issued by a valid and
     * trusted CA) 
     */
    bool validate();

    /**
     * Check the certificate in cert against the CA certificates in
     * certfile - returns true if cert was signed by one of the CA
     * certificates.
     */
    bool signedbyCAindir(WvString certdir);

    /**
     * Check the certificate in cert against the CA certificates in certdir
     * - returns true if cert was signed by one of the CA certificates. 
     */
    bool signedbyCAinfile(WvString certfile);

    /**
     * Sign the X509 certificate in cert with CAKeypair
     */
    void signcert(WvRSAKey CAKeypair);

    /**
     * Check and see if the certificate in cert has been revoked... currently
     * relies on the CRL Distribution Point X509v3 extension...
     * returns true if it has expired
     * 
     * NOT IMPLEMENTED
     */
    bool isinCRL();
        
    /**
     * Dump the X509 Certificate in cert to outfile in PEM
     */
    void dumpcert(WvString outfile, bool append = false);
    
    /**
     * Dump RSA Keypair to outfile in PEM format 
     */
    void dumpkeypair(WvString outfile, bool append = false);

    /**
     * Dump RSA Keypair to outfile in RAW format (suitable for FreeS/WAN)
     */
    void dumprawkeypair(WvString outfile, bool append = false);

    WvLog debug;
    
    WvString errstring;
    
    bool isok() const
        { return cert && rsa && !errstring; }
    const WvString &errstr()
        { return errstring; }
    
    void seterr(WvStringParm s)
        { errstring = s; }
    void seterr(WVSTRING_FORMAT_DECL)
        { seterr(WvString(WVSTRING_FORMAT_CALL)); }
};

#endif // __WVX509_H
