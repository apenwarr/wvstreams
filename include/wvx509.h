/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2004 Net Integration Technologies, Inc.
 *
 * X.509 certificate management classes.
 */ 
#ifndef __WVX509_H
#define __WVX509_H

#include "wvlog.h"
#include "wverror.h"
#include "wvstringlist.h"

// Structures to make the compiler happy so we don't have to include x509v3.h ;)
struct x509_st;
typedef struct x509_st X509;
struct ssl_ctx_st;
typedef struct ssl_ctx_st SSL_CTX;

struct X509_name_st;
typedef struct X509_name_st X509_NAME;

struct asn1_string_st;
typedef struct asn1_string_st ASN1_TIME;

class WvRSAKey;
class WvCRL;

// workaround for the fact that OpenSSL initialization stuff must be called
// only once.
void wvssl_init();
void wvssl_free();
WvString wvssl_errstr();


/**
 * X509 Class to handle certificates and their related
 * functions
 */
class WvX509Mgr : public WvError, public IObject
{
    IMPLEMENT_IOBJECT(WvX509Mgr);
public:
   /**
    * Type for the @ref encode() and @ref decode() methods.
    * CertPEM   = PEM Encoded X.509 Certificate
    * CertDER   = DER Encoded X.509 Certificate
    * CertDER64 = DER Encoded X.509 Certificate returned in Base64
    * CertSMIME = SMIME "Certificate" usable for userSMIMECertificate ldap entry 
    *             again in Base64
    * RsaPEM    = PEM Encoded RSA Private Key
    * RsaPubPEM = PEM Encoded RSA Public Key
    * RsaRaw    = Raw form of RSA Key (unused by most programs, FreeS/WAN
    * being the notable exception)
    */
    enum DumpMode { CertPEM = 0, CertDER, CertDER64, RsaPEM, RsaPubPEM, RsaRaw };


    /**
     * Initialize a completely empty X509 Object with an X509 certificate
     * that doesn't have anything it it... good for building custom 
     * certificates.
     */
    WvX509Mgr();
    
    /**
     * Initialize a blank X509 Object with the certificate *cert
     * (used for client side operations...)
     * 
     * This either initializes a completely empty object, or takes _cert,
     * and extracts the distinguished name into dname, and the the RSA
     * public key into rsa. rsa->prv is empty.
     */
    WvX509Mgr(X509 *_cert);

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
     * RSA key; 1024 or 2048 are good values for this. If 'ca' is true, the
     * certificate will be created as a certificate authority.
     */
    WvX509Mgr(WvStringParm _dname, int bits, bool isca=false);

private:
    /** 
     * Placeholder for Copy Constructor: this doesn't exist yet, but it keeps
     * us out of trouble :) 
     */
    WvX509Mgr(const WvX509Mgr &mgr);

public:
    /** Destructor */
    virtual ~WvX509Mgr();
    
    /**
     * Avoid a lot of ugliness by having it so that we are binding to the SSL
     * context, and not the other way around, since that would make ownership
     * of the cert and rsa keys ambiguous.
     */
    bool bind_ssl(SSL_CTX *ctx);
 
    /**
     * Accessor for the RSA Keys
     */
    const WvRSAKey &get_rsa();
    
    /**
     * Allow us to access the certificate member - this will be going away 
     * eventually, but for now, it gets us out of a couple of issues :/
     */
    X509 *get_cert() const { return cert; }

    /**
     * Set the public key of the certificate to the public key rsa_pubkey.
     * Does NOT affect the rsa member... (FIXME!)
     */
    void set_pubkey(WvRSAKey *rsa_pubkey);
    
    /**
     * Given the Distinguished Name dname and an already generated keypair in 
     * rsa, return a Self Signed Certificate in cert.
     * If is_ca, it will generate a self signed certificate with the 
     * appropriate values for a certificate authority (or at least the most
     * common ones).
     */
    void create_selfsigned(bool is_ca = false);

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
    static WvString certreq(WvStringParm subject, const WvRSAKey &rsa);
    
    /**
     * Take the PKCS#10 request in the string pkcs10req, sign it with the
     * private key in rsa, and then spit back a new X509 Certificate in
     * PEM format.
     */
    WvString signreq(WvStringParm pkcs10req);

    /**
     * Sign the certificate with our keys... this is what you want when 
     * the signcert above doesn't set up the various parameters that you
     * may need in your environment.
     */
    bool signcert(X509 *cert);
    
    /**
     * Take the CRL in crl, and sign it. returns true if successfull, and 
     * false if not. If false, check crl.err.geterr() for reason.
     */
    bool signcrl(WvCRL *crl);

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
     * you can always call get_rsa().private_str() and get_rsa().public_str()
     * for that information.
     */
    WvString hexify();

    /**
     * Function to verify the validity of a certificate that has been
     * placed in cert. It can check and make sure that it was signed by
     * the CA certificate cacert, and is not in the CRL crl, but at the
     * very least, it checks and makes sure that your certificate is not 
     * expired
     */
    bool validate(WvX509Mgr *cacert = NULL, WvCRL *crl = NULL);

   /**
    * Check the certificate in cert against the CA certificate in cacert
    * - returns true if cert was signed by that CA certificate.
    */
    bool signedbyca(WvX509Mgr *cacert);

    /**
     * Check to see if the certificate in cert was issued by the CA
     * certificate in cacert. Note: You are going on the certificate's
     * say-so by using this function. You may also want to use signedbyca
     * to check if the certificate is actually signed by who it claims
     * to be issued by.
     */
    bool issuedbyca(WvX509Mgr *cacert);

    /**
     * Sign the contents of data and return the signature as a BASE64
     * string.
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

    void load(const DumpMode mode, WvStringParm fname);
    
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
    void decode(const DumpMode mode, WvStringParm encoded);
    void decode(const DumpMode mode, WvBuf &encoded);

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
     * Get and set the Certificate Issuer (usually the CA who signed 
     * the certificate).
     */
    WvString get_issuer();
    void set_issuer(WvStringParm name);
    
    /**
     * get and set the Subject field of the certificate
     */
    WvString get_subject();
    void set_subject(WvStringParm name);
    void set_subject(X509_NAME *name);
    /**
     * get and set the serialNumber field of the certificate
     */
    WvString get_serial();
    void set_serial(long serial_no);

    /** 
     * get and set the Netscape Comment extension
     */
    WvString get_nscomment();
    void set_nscomment(WvStringParm comment);
    
    /**
     * get and set the Netscape SSL Server extension
     */
    WvString get_nsserver();
    void set_nsserver(WvStringParm server_fqdn);
    
    /**
     * get the CRL Distribution points if they exist, WvString::null
     * if they don't.
     */
    WvString get_crl_dp();

    /**
     * Get any certificate Policy OIDs.
     */
    void get_cp_oids(WvStringList &oids);
    
    /**
     * Set the Certificate Policy OID from the string given by OID
     * i.e: 1.2.3.4.5.6.7.8, and an optional URL that points to it's
     * CPS.
     */
    void set_cp_oid(WvStringParm oid, WvStringParm url);


    /**
     * Set the Certificate to use X509v3, since that's all modern
     * PKI uses anyways :)
     */
    void set_version();

    /**
     * Get and set the keyUsage field.
     */
    WvString get_key_usage();
    void set_key_usage(WvStringParm values);

    /**
     * Get and set the extendedKeyUsage field.
     */
    WvString get_ext_key_usage();
    void set_ext_key_usage(WvStringParm values);
    
    /**
     * Return the Subject alt name if it exists, and WvString::null if
     * it doesn't.
     */
    WvString get_altsubject();

    /**
     * Set the Subject Alt Name.
     */
    void set_altsubject(WvStringParm name);
    
    /**
     * Get and Set the Policy Constraints extension
     */
    WvString get_constraints();
    void set_constraints(WvStringParm constraint);
    
    /**
     * Return the not before and not after in a format we're more able to easily use.
     */
    time_t get_notvalid_before();
    time_t get_notvalid_after();
    
    /**
     * Set the lifetime to be used for this certificate... the lifetime starts
     * from the minute that the certificate is signed...
     */
    void set_lifetime(long seconds);
    
    /**
     * Get the authority info access information. Usually includes a list of URLs
     * where the issuer's CA certificate may be fetched, as well as a list of
     * OCSP responders. Note that this function returns this information in
     * a giant string: get_ca_urls and get_ocsp may return this information in a 
     * more useful format.
     */
    WvString get_aia();

    /**
     * Set a list of urls that have the Certificate of the CA that issued 
     * this certificate, as well as the list of OCSP responders for this
     * certificate.
     */
    void set_aia(WvStringList &ca_urls, WvStringList &responders);

    /**
     * Get a list of OCSP Responders for this certificate
     */
     
    void get_ocsp(WvStringList &responders);
    
    /**
     * Get a list of urls that have the Certificate 
     * of the CA that issued this certificate
     */
    void get_ca_urls(WvStringList &urls);
    
    /**
     * Get a list of URLs that are valid CRL distribution
     * points for this certificate.
     */
    void get_crl_urls(WvStringList &urls);

    /**
     * Set the list of URLs that are valid CRL distribution
     * points for this certificate.
     */
    void set_crl_urls(WvStringList &urls);

    /**
     * Get the Subject Key Info
     */
    WvString get_ski();
    
    /**
     * Get the Authority key Info
     */
    WvString get_aki();

    void set_dname(WvStringParm _dname)
    {	dname = _dname; }

    // Takes ownership..
    // Fixme: Implement RefCounting in WvRSAKey!
    void set_rsakey(WvRSAKey *_rsa)
    {   rsa = _rsa; }

    /**
     * Is this certificate Object valid, and in a non-error state
     */
    virtual bool isok() const;

    virtual WvString errstr() const;

    virtual int geterr() const;

private:
    friend class WvCRL;

    /** X.509v3 Certificate - this is why this class exists */
    X509     *cert;

    /**
     * The Public and Private RSA keypair associated with this certificate
     * Make sure that you save this somewhere!!! If you don't, then you won't
     * really be able to use the certificate for anything...
     */
    WvRSAKey *rsa;
    
    /** Distinguished Name to be used in the certificate. */
    WvString dname;

    WvLog debug;
    
    /** 
    * Password for PKCS12 dump - we don't handle this entirely correctly 
    * since we should erase it from memory as soon as we are done with it
    */
    WvString pkcs12pass;

    /**
     * Get and the Extension information - returns NULL if extension doesn't exist
     * Used internally by all of the get_??? and set_??? functions (crl_dp, cp_oid, etc.).
     */
    WvString get_extension(int nid);
    void set_extension(int nid, WvStringParm values);
    
    /**
     * Populate dname (the distinguished name);
     */
    void filldname();

    /**
     * Create an X509 certificate from an ASN1-encoded file
     */
    void load_asn1_from_file(WvStringParm fname);

    /**
     * Create an X509 certificate from a PEM-encoded file
     */
    void load_pem_from_file(WvStringParm fname);

    /**
     * Return a WvRSAKey filled with the public key from the
     * certificate in cert
     */
    WvRSAKey *fillRSAPubKey();
};

#endif // __WVX509_H
