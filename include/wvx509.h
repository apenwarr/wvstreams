/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * X.509 certificate management classes.
 */ 
#ifndef __WVX509_H
#define __WVX509_H

#include "wvrsa.h"
#include "wvlog.h"
#include "wverror.h"

// Structures to make the compiler happy so we don't have to include x509v3.h ;)
struct x509_st;
typedef struct x509_st X509;

// workaround for the fact that OpenSSL initialization stuff must be called
// only once.
void wvssl_init();
void wvssl_free();
WvString wvssl_errstr();


/**
 * X509 Class to handle certificates and their related
 * functions
 */
class WvX509Mgr : public WvError
{
public:
   /** Distinguished Name to be used in the certificate. */
    WvString dname;

   /**
    * Type for the @ref encode() and decode() methods.
    * CertPEM   = PEM Encoded X.509 Certificate
    * RsaPEM    = PEM Encoded RSA Private Key
    * RsaPubPEM = PEM Encoded RSA Public Key
    * RsaRaw    = Raw form of RSA Key (unused by most programs, FreeS/WAN
    * being the notable exception)
    */
    enum DumpMode { CertPEM = 0, RsaPEM, RsaPubPEM, RsaRaw };

   /**
    * Initialize a blank X509 Object with the certificate *cert
    * (used for client side operations...)
    * 
    * This either initializes a completely empty object, or takes _cert,
    * and extracts the distinguished name into dname, and the the RSA
    * public key into rsa. rsa->prv is empty.
    */
    WvX509Mgr(X509 *_cert = NULL);

    /** 
     * Constructor to initialize this object with a pre-existing 
     * certificate and key 
     */
    WvX509Mgr(WvStringParm hexcert, WvStringParm hexrsa);

    /**
     * Constructor to create a self-signed certificate for the given dn and
     * RSA key.  If you don't already have a WvRSAKey, try the other
     * constructor, below, which creates one automatically.
     * 
     * For SSL Servers, the dname must contain a "cn=" section in order to
     * validate correctly with some clients, particularly web browsers.
     * For example, if your domain name is nit.ca, you can try this for
     * _dname: "cn=nit.ca,o=Net Integration,c=CA", or maybe this instead:
     * "cn=nit.ca,dc=nit,dc=ca"
     * 
     * We don't check automatically that your _dname complies with these
     * restrictions, since non-SSL certificates may be perfectly valid
     * without this.  If you want to generate invalid certs, that's up to
     * you.
     */
    WvX509Mgr(WvStringParm _dname, WvRSAKey *_rsa);
    
    /**
     * Constructor to create a new self-signed certificate for the given dn
     * and number of bits.  See the previous constructor for details on how
     * to choose _dname.  'bits' is the number of bits in the auto-generated
     * RSA key; 1024 or 2048 are good values for this.
     */
    WvX509Mgr(WvStringParm _dname, int bits);

private:
    /** Placeholder: this doesn't exist yet. */
    WvX509Mgr(const WvX509Mgr &mgr);

public:
    /** Destructor */
    virtual ~WvX509Mgr();

    /** X.509v3 Certificate - this is why this class exists */
    X509     *cert;

    /**
     * The Public and Private RSA keypair associated with this certificate
     * Make sure that you save this somewhere!!! If you don't, then you won't
     * really be able to use the certificate for anything...
     */
    WvRSAKey *rsa;
    
    /**
     * Given the Distinguished Name dname and an already generated keypair in 
     * rsa, return a Self Signed Certificate in cert.
     */
    void create_selfsigned();

    /**
     * Create a certificate request (PKCS#10) using this function.. this 
     * request is what you would send off to Verisign, or Entrust.net (or any
     * other CA), to get your real certificate. It leaves the RSA key pair
     * in rsa, where you MUST save it for the certificate to be AT ALL
     * valid when you get it back. Returns a PEM Encoded PKCS#10 certificate
     * request, and leaves the RSA keypair in rsa, and a self-signed temporary
     * certificate in cert.
     * 
     * It uses dname as the Distinguished name to create this Request.
     * Make sure that it has what you want in it first.
     */    
    WvString certreq();
    
    /**
     * Test to make sure that a certificate and a keypair go together.
     * called internally by unhexify() although you can call it if 
     * you want to test a certificate yourself. (Such as after a decode)
     */
    bool test();

    /**
     * Given a hexified certificate, fill the cert member NOTE: ALWAYS load
     * your RSA Keys before calling this! It is best if you have hexify()'d
     * keys to simply use the proper constructor. 
     */
    void unhexify(WvStringParm encodedcert);
    
    /**
     * Given the X509 certificate object cert, return a hexified string
     * useful in a WvConf or UniConf file.
     * 
     * I don't provide a similar function for that for the rsa key, because
     * you can always call rsa->private_str() and rsa->public_str()
     * for that information.
     */
    WvString hexify();

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
     * certdir - returns true if cert was signed by one of the CA
     * certificates.
     */
    bool signedbyCAindir(WvStringParm certdir);
   
    /**
     * Check the certificate in cert against the CA certificate in certfile
     * - returns true if cert was signed by that CA certificate. 
     */
   bool signedbyCAinfile(WvStringParm certfile);

    /**
     * Sign the contents of data and return the signature.
     */
    WvString sign(WvBuf &data);
    WvString sign(WvStringParm data);

    /**
     * Verify that the contents of data were signed
     * by the certificate currently in cert. This only
     * checks the signature, it doesn't check the validity
     * of the certificate.
     */
    bool verify(WvBuf &original, WvStringParm signature);
    bool verify(WvStringParm original, WvStringParm signature);
    
    /**
     * Check and see if the certificate in cert has been revoked... currently
     * relies on the CRL Distribution Point X509v3 extension...
     * returns true if it has expired
     * 
     * NOT IMPLEMENTED
     */
    bool isinCRL();

    /** Return the information requested by mode as a WvString. */
    WvString encode(const DumpMode mode);

    /**
     * Load the information from the format requested by mode into
     * the class - this overwrites the certificate, and possibly the
     * key - and to enable two stage loading (the certificate first, then the
     * key), it DOES NOT call test() - that will be up to the programmer
     */
    void decode(const DumpMode mode, WvStringParm PemEncoded);

    /**
     * And of course, since PKCS12 files are in the rediculous DER encoding 
     * format, which is binary, we can't use the encode/decode functions, so
     * we deal straight with files... *sigh*
     * 
     * As should be obvious, this writes the certificate and RSA keys in PKCS12
     * format to the file specified by filename.
     */
    void write_p12(WvStringParm filename);
    
    /**
     * And this reads from the file specified in filename, and fills the RSA and
     * cert members with the decoded information.
     */
    void read_p12(WvStringParm filename);

    /** Sets the PKCS12 password */
    void setPkcs12Password(WvStringParm passwd)
    	{ pkcs12pass = passwd; }

    /** 
     * Return the Certificate Issuer (usually the CA who signed 
     * the certificate)
     */
    WvString get_issuer();

    /**
     * Return the Subject field of the certificate
     */
    WvString get_subject();

    /**
     * Return the CRL Distribution points if they exist, WvString::null
     * if they don't.
     */
    WvString get_crl_dp();

    /**
     * Return the Certificate Policy OID if it exists, and WvString::null
     * it if doesn't.
     */
    WvString get_cp_oid();

    /**
     * Return the Subject alt name if it exists, and WvString::null if
     * it doesn't.
     */
    WvString get_altsubject();

    /**
     * Is this certificate Object valid, and in a non-error state
     */
    virtual bool isok() const;

    virtual WvString errstr() const;

    virtual int geterr() const;

private:
    WvLog debug;

   /** 
    * Password for PKCS12 dump - we don't handle this entirely correctly 
    * since we should erase it from memory as soon as we are done with it
    */
    WvString pkcs12pass;

    /**
     * Get the Extension information - returns NULL if extension doesn't exist
     */
    WvDynBuf *get_extension(int nid);

    /**
     * Populate dname (the distinguished name);
     */
    void filldname();

    /**
     * Return a WvRSAKey filled with the public key from the
     * certificate in cert
     */
    WvRSAKey *fillRSAPubKey();
};

#endif // __WVX509_H
