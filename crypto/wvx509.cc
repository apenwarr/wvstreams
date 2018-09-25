/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2005 Net Integration Technologies, Inc.
 * 
 * X.509 certificate management classes.
 */ 
#include "wvx509.h"
#include "wvcrl.h"
#include "wvsslhacks.h"
#include "wvcrypto.h"
#include "wvstringlist.h"
#include "wvbase64.h"
#include "wvstrutils.h"
#include "wvautoconf.h"

#include <openssl/pem.h>
#include <openssl/x509v3.h>
#include <openssl/err.h>
#include <openssl/sha.h>
#include <openssl/ssl.h>

// enable this to add some extra debugging trace messages (this can be VERY
// verbose)
#if 0
# define TRACE(x, y...) debug(x, ## y); 
#else
#ifndef _MSC_VER
# define TRACE(x, y...)
#else
# define TRACE
#endif
#endif

// helper method to let us return and warn gracefully when getting/setting an 
// element in a null certificate
static const char * warning_str_set 
    = "Tried to set %s, but certificate not ok.\n";
static const char * warning_str_get 
    = "Tried to get %s, but certificate not ok.\n";
#define CHECK_CERT_EXISTS_SET(x)                                        \
    if (!cert) {                                                        \
        debug(WvLog::Warning, warning_str_set, x);                      \
        return;                                                         \
    }
#define CHECK_CERT_EXISTS_GET(x, y)                                     \
    if (!cert) {                                                        \
        debug(WvLog::Warning, warning_str_get, x);                      \
        return y;                                                       \
    }
        

UUID_MAP_BEGIN(WvX509)
  UUID_MAP_ENTRY(IObject)
  UUID_MAP_END

static int ssl_init_count = 0;

#if !HAVE_OPENSSL_POLICY_MAPPING

// HACK: old versions of OpenSSL can't handle ERR_free_strings() being called
// more than once in the same process; the next wvssl_init won't work.  So
// let's make sure to make a global variable that holds the refcount at 1
// even when all the objects go away, then clean it up at exit.
class WvSSL_Stupid_Refcount
{
public:
   WvSSL_Stupid_Refcount()
   {
       wvssl_init();
   }
   
   ~WvSSL_Stupid_Refcount()
   {
       wvssl_free();
   }
};

WvSSL_Stupid_Refcount wvssl_stupid_refcount;

#endif // !HAVE_OPENSSL_POLICY_MAPPING


void wvssl_init()
{
    if (!ssl_init_count)
    {
	SSL_library_init();
	SSL_load_error_strings();
	ERR_load_BIO_strings();
	ERR_load_crypto_strings();
	OpenSSL_add_all_algorithms();
	OpenSSL_add_all_ciphers();
	OpenSSL_add_all_digests();
    }
    
    ssl_init_count++;
}


void wvssl_free()
{
    assert(ssl_init_count >= 1);
    if (ssl_init_count >= 1)
	ssl_init_count--;

    if (!ssl_init_count)
    {
	ERR_free_strings();
	EVP_cleanup();
    }
}


WvString wvssl_errstr()
{
    char buf[256];
    ERR_error_string_n(ERR_get_error(), buf, sizeof(buf));
    buf[sizeof(buf)-1] = 0;
    return buf;
}


WvX509::WvX509(X509 *_cert)
    : debug("X509", WvLog::Debug5)
{
    wvssl_init();
    cert = _cert;
}


WvX509::WvX509()
    : debug("X509", WvLog::Debug5)
{
    wvssl_init();
    cert = NULL;
}


WvX509::WvX509(const WvX509 &x509)
    : debug("X509", WvLog::Debug5)
{
    wvssl_init();
    if (x509.cert)
        cert = X509_dup(x509.cert);
    else
        cert = NULL;
}


WvX509::~WvX509()
{
    TRACE("Deleting.\n");
    
    if (cert)
	X509_free(cert);

    wvssl_free();
}



// The people who designed this garbage should be shot!
// Support old versions of openssl...
#ifndef NID_domainComponent
#define NID_domainComponent 391
#endif

#ifndef NID_Domain
#define NID_Domain 392
#endif


// returns some approximation of the server's fqdn, or an empty string.
static WvString set_name_entry(X509_NAME *name, WvStringParm dn)
{
    WvString fqdn(""), force_fqdn("");
    X509_NAME_ENTRY *ne = NULL;
    int count = 0, nid;
    
    WvStringList l;
    l.split(dn, ",");
    
    // dn is of the form: c=ca,o=foo organization,dc=foo,dc=com
    // (ie. name=value pairs separated by commas)
    WvStringList::Iter i(l);
    for (i.rewind(); i.next(); )
    {
	WvString s(*i), sid;
	char *cptr, *value;
	
	cptr = s.edit();
	value = strchr(cptr, '=');
	if (value)
	    *value++ = 0;
	else
	    value = (char*)"NULL";
	
	sid = strlwr(trim_string(cptr));
	
	if (sid == "c")
	    nid = NID_countryName;
	else if (sid == "st")
	    nid = NID_stateOrProvinceName;
	else if (sid == "l")
	    nid = NID_localityName;
	else if (sid == "o")
	    nid = NID_organizationName;
	else if (sid == "ou")
	    nid = NID_organizationalUnitName;
	else if (sid == "cn")
	{
	    nid = NID_commonName;
	    force_fqdn = value;
	}
	else if (sid == "dc")
	{
	    nid = NID_domainComponent;
	    if (!!fqdn)
		fqdn.append(".");
	    fqdn.append(value);
	}
	else if (sid == "domain")
	{
	    nid = NID_Domain;
	    force_fqdn = value;
	}
	else if (sid == "email")
	    nid = NID_pkcs9_emailAddress;
	else
	    nid = NID_domainComponent;
	
	// Sometimes we just want to parse dn into fqdn.
	if (name == NULL)
	    continue;
	
	if (!ne)
	    ne = X509_NAME_ENTRY_create_by_NID(NULL, nid,
			       V_ASN1_APP_CHOOSE, (unsigned char *)value, -1);
	else
	    X509_NAME_ENTRY_create_by_NID(&ne, nid,
			       V_ASN1_APP_CHOOSE, (unsigned char *)value, -1);
	if (!ne)
	    continue;
	
	X509_NAME_add_entry(name, ne, count++, 0);
    }
    
    X509_NAME_ENTRY_free(ne);
    
    if (!!force_fqdn)
	return force_fqdn;

    return fqdn;
}


WvRSAKey *WvX509::get_rsa_pub() const
{
    EVP_PKEY *pkcert = X509_get_pubkey(cert);
    RSA *certrsa = EVP_PKEY_get1_RSA(pkcert);
    EVP_PKEY_free(pkcert);
    return new WvRSAKey(certrsa, false); 
}


WvString WvX509::certreq(WvStringParm subject, const WvRSAKey &rsa)
{
    WvLog debug("X509::certreq", WvLog::Debug5);

    EVP_PKEY *pk = NULL;
    X509_NAME *name = NULL;
    X509_REQ *certreq = NULL;

    // double check RSA key
    if (rsa.isok())
	debug("RSA Key is fine.\n");
    else
    {
	debug(WvLog::Warning, "RSA Key is bad");
	return WvString::null;
    }

    if ((pk=EVP_PKEY_new()) == NULL)
    {
        debug(WvLog::Warning,
	      "Error creating key handler for new certificate");
        return WvString::null;
    }
    
    if ((certreq=X509_REQ_new()) == NULL)
    {
        debug(WvLog::Warning, "Error creating new PKCS#10 object");
	EVP_PKEY_free(pk);
        return WvString::null;
    }

    if (!EVP_PKEY_set1_RSA(pk, rsa.rsa))
    {
        debug(WvLog::Warning, "Error adding RSA keys to certificate");
	X509_REQ_free(certreq);
	EVP_PKEY_free(pk);
        return WvString::null;
    }
    
    X509_REQ_set_version(certreq, 0); /* version 1 */

    X509_REQ_set_pubkey(certreq, pk);

    name = X509_REQ_get_subject_name(certreq);

    debug("Creating Certificate request for %s\n", subject);
    set_name_entry(name, subject);
    X509_REQ_set_subject_name(certreq, name);
    char *sub_name = X509_NAME_oneline(X509_REQ_get_subject_name(certreq), 
				       0, 0);
    debug("SubjectDN: %s\n", sub_name);
    OPENSSL_free(sub_name);
    
    if (!X509_REQ_sign(certreq, pk, EVP_sha1()))
    {
	debug(WvLog::Warning, "Could not self sign the request");
	X509_REQ_free(certreq);
	EVP_PKEY_free(pk);
        return WvString::null;
    }

    int verify_result = X509_REQ_verify(certreq, pk);
    if (verify_result == 0)
    {
	debug(WvLog::Warning, "Self signed request failed");
	X509_REQ_free(certreq);
	EVP_PKEY_free(pk);
        return WvString::null;
    }
    else
    {
	debug("Self Signed Certificate Request verifies OK!\n");
    }

    // Horribly involuted hack to get around the fact that the
    // OpenSSL people are too braindead to have a PEM_write function
    // that returns a char *
    WvDynBuf retval;
    BIO *bufbio = BIO_new(BIO_s_mem());
    BUF_MEM *bm;
    
    PEM_write_bio_X509_REQ(bufbio, certreq);
    BIO_get_mem_ptr(bufbio, &bm);
    retval.put(bm->data, bm->length);
    
    X509_REQ_free(certreq);
    EVP_PKEY_free(pk);
    BIO_free(bufbio);

    return retval.getstr();
}


bool WvX509::validate(WvX509 *cacert) const
{
    if (cert == NULL)
    {
        debug(WvLog::Warning, "Tried to validate certificate against CA, but "
              "certificate is blank!\n");
        return false;
    }

    bool retval = true;

    // Check and make sure that the certificate is still valid
    if (X509_cmp_current_time(X509_get_notAfter(cert)) < 0)
    {
        debug("Certificate has expired.\n");
        retval = false;
    }
    
    if (X509_cmp_current_time(X509_get_notBefore(cert)) > 0)
    {
        debug("Certificate is not yet valid.\n");
        retval = false;
    }

    if (cacert)
    {
        retval &= signedbyca(*cacert);
        retval &= issuedbyca(*cacert);
    }
    
    return retval;
}


bool WvX509::signedbyca(WvX509 &cacert) const
{
    if (!cert || !cacert.cert)
    {
        debug(WvLog::Warning, "Tried to determine if certificate was signed "
              "by CA, but either client or CA certificate (or both) are "
              "blank.\n");
        return false;
    } 

    EVP_PKEY *pkey = X509_get_pubkey(cacert.cert);
    int result = X509_verify(cert, pkey); 
    EVP_PKEY_free(pkey);

    if (result < 0)
    {
        debug("Can't determine if we were signed by CA %s: %s\n",
              cacert.get_subject(), wvssl_errstr());
        return false;
    }
    bool issigned = (result > 0);

    debug("Certificate was%s signed by CA %s.\n", issigned ? "" : " NOT", 
          cacert.get_subject());

    return issigned;
}


bool WvX509::issuedbyca(WvX509 &cacert) const
{
    if (!cert || !cacert.cert)
    {
        debug(WvLog::Warning, "Tried to determine if certificate was issued "
              "by CA, but either client or CA certificate (or both) are "
              "blank.\n");
        return false;
    } 

    int ret = X509_check_issued(cacert.cert, cert);
    debug("issuedbyca: %s==X509_V_OK(%s)\n", ret, X509_V_OK);
    if (ret != X509_V_OK)
	return false;

    return true;
}


WvString WvX509::encode(const DumpMode mode) const
{
    WvDynBuf retval;
    encode(mode, retval);
    return retval.getstr();
}


void WvX509::encode(const DumpMode mode, WvBuf &buf) const
{
    if (mode == CertFileDER || mode == CertFilePEM)
        return; // file modes are no ops with encode

    if (!cert)
    {
        debug(WvLog::Warning, "Tried to encode certificate, but certificate "
              "is blank!\n");
        return;
    }

    debug("Encoding X509 certificate.\n");

    if (mode == CertHex)
    {
        size_t size;
        unsigned char *keybuf, *iend;
        WvString enccert;
        
        size = i2d_X509(cert, NULL);
        iend = keybuf = new unsigned char[size];
        i2d_X509(cert, &iend);
        
        enccert.setsize(size * 2 +1);
        ::hexify(enccert.edit(), keybuf, size);
        
        deletev keybuf;
        buf.putstr(enccert);
    }
    else
    {
        BIO *bufbio = BIO_new(BIO_s_mem());
        BUF_MEM *bm;
        
        if (mode == CertPEM)
            PEM_write_bio_X509(bufbio, cert);
        else if (mode == CertDER)
            i2d_X509_bio(bufbio, cert);
        else
            debug(WvLog::Warning, "Tried to encode certificate with unknown "
                  "mode!\n");

        BIO_get_mem_ptr(bufbio, &bm);
        buf.put(bm->data, bm->length);
        BIO_free(bufbio);
    }
}


void WvX509::decode(const DumpMode mode, WvStringParm str)
{
    if (cert)
    {
        debug("Replacing an already existant X509 certificate.\n");
        X509_free(cert);
        cert = NULL;
    }

    if (mode == CertFileDER)
    {
        BIO *bio = BIO_new(BIO_s_file());
        
        if (BIO_read_filename(bio, str.cstr()) <= 0)
        {
            debug(WvLog::Warning, "Open '%s': %s\n", str, wvssl_errstr());
            BIO_free(bio);
            return;
        }
        
        if (!(cert = d2i_X509_bio(bio, NULL)))
            debug(WvLog::Warning, "Import DER from '%s': %s\n",
		  str, wvssl_errstr());
        
        BIO_free(bio);
        return;
    }
    else if (mode == CertFilePEM)
    {
        FILE *fp = fopen(str, "rb");
        if (!fp)
        {
	    int errnum = errno;
            debug("Open '%s': %s\n", str, strerror(errnum));
            return;
        }

        if (!(cert = PEM_read_X509(fp, NULL, NULL, NULL)))
            debug(WvLog::Warning, "Import PEM from '%s': %s\n",
		  str, wvssl_errstr());
        
        fclose(fp);
        return;
    }
    else if (mode == CertHex)
    {
        int hexbytes = str.len();
        int bufsize = hexbytes/2;
        unsigned char *certbuf = new unsigned char[bufsize];
        unsigned char *cp = certbuf;
        X509 *tmpcert;
        
        ::unhexify(certbuf, str);
        tmpcert = cert = X509_new();
        cert = wv_d2i_X509(&tmpcert, &cp, bufsize);    
        delete[] certbuf;
        return;
    }

    // we use the buffer decode functions for everything else
    WvDynBuf buf;
    buf.putstr(str);
    decode(mode, buf);
}


void WvX509::decode(const DumpMode mode, WvBuf &encoded)
{
    if (cert)
    {
        debug("Replacing an already existant X509 certificate.\n");
        X509_free(cert);
        cert = NULL;
    }
    
    if (mode == CertHex || mode == CertFileDER || mode == CertFilePEM)
        decode(mode, encoded.getstr());
    else
    {        
        BIO *membuf = BIO_new(BIO_s_mem());
        size_t len = encoded.used();
        BIO_write(membuf, encoded.get(len), len);

        if (mode == CertPEM)
            cert = PEM_read_bio_X509(membuf, NULL, NULL, NULL);
        else if (mode == CertDER)
            cert = d2i_X509_bio(membuf, NULL);
        else
            debug(WvLog::Warning, "Tried to decode certificate with unknown "
                  "mode!\n");

        BIO_free_all(membuf);
    }
}


WvString WvX509::get_issuer() const
{ 
    CHECK_CERT_EXISTS_GET("issuer", WvString::null);

    char *name = X509_NAME_oneline(X509_get_issuer_name(cert),0,0);
    WvString retval(name);
    OPENSSL_free(name);
    return retval;
}


void WvX509::set_issuer(WvStringParm issuer)
{
    CHECK_CERT_EXISTS_SET("issuer");

    X509_NAME *name = X509_get_issuer_name(cert);
    set_name_entry(name, issuer);
    X509_set_issuer_name(cert, name);
}


void WvX509::set_issuer(const WvX509 &cacert)
{
    CHECK_CERT_EXISTS_SET("issuer");

    X509_NAME *casubj = X509_get_subject_name(cacert.cert);
    X509_set_issuer_name(cert, casubj);
}


WvString WvX509::get_subject() const
{
    CHECK_CERT_EXISTS_GET("subject", WvString::null);

    char *name = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
    WvString retval(name);
    OPENSSL_free(name);
    return retval;
}


void WvX509::set_subject(WvStringParm subject)
{    
    CHECK_CERT_EXISTS_SET("subject");

    X509_NAME *name = X509_get_subject_name(cert);
    set_name_entry(name, subject);
    X509_set_subject_name(cert, name);
}


void WvX509::set_subject(X509_NAME *name)
{
    CHECK_CERT_EXISTS_SET("subject");

    X509_set_subject_name(cert, name);
}


void WvX509::set_pubkey(WvRSAKey &_rsa)
{
    CHECK_CERT_EXISTS_SET("pubkey");

    EVP_PKEY *pk = EVP_PKEY_new();
    assert(pk);

    // Assign RSA Key from WvRSAKey into stupid package that OpenSSL needs
    if (!EVP_PKEY_set1_RSA(pk, _rsa.rsa))
    {
	debug("Error adding RSA keys to certificate.\n");
	return;
    }
    
    X509_set_pubkey(cert, pk);

    EVP_PKEY_free(pk);
}



void WvX509::set_nsserver(WvStringParm servername)
{
    CHECK_CERT_EXISTS_SET("nsserver");
    
    WvString fqdn;
    
    // FQDN cannot have a = in it, therefore it
    // must be a distinguished name :)
    if (strchr(servername, '='))
	fqdn = set_name_entry(NULL, servername);
    else
	fqdn = servername;
    
    if (!fqdn)
	fqdn = "null.noname.null";
    
    debug("Setting Netscape SSL server name extension to '%s'.\n", fqdn);

    // Add in the netscape-specific server extension
    set_extension(NID_netscape_cert_type, "server");
    set_extension(NID_netscape_ssl_server_name, fqdn);
}


WvString WvX509::get_nsserver() const
{
    return get_extension(NID_netscape_ssl_server_name);
}


WvString WvX509::get_serial(bool hex) const
{
    CHECK_CERT_EXISTS_GET("serial", WvString::null);

    BIGNUM *bn = BN_new();
    bn = ASN1_INTEGER_to_BN(X509_get_serialNumber(cert), bn);
    char * c;
    if (hex)
        c = BN_bn2hex(bn);
    else
        c = BN_bn2dec(bn);
    WvString ret("%s", c);
    OPENSSL_free(c);
    BN_free(bn);
    return ret;
}


void WvX509::set_version()
{
    CHECK_CERT_EXISTS_SET("version");

    X509_set_version(cert, 0x2);
}


void WvX509::set_serial(long serial)
{
    CHECK_CERT_EXISTS_SET("serial");

    ASN1_INTEGER_set(X509_get_serialNumber(cert), serial);
}


WvString WvX509::get_crl_dp() const
{
    return get_extension(NID_crl_distribution_points);
}


void WvX509::set_lifetime(long seconds)
{
    CHECK_CERT_EXISTS_SET("lifetime");

    // Set the NotBefore time to now.
    X509_gmtime_adj(X509_get_notBefore(cert), 0);
    
    // Now + 10 years... should be shorter, but since we don't currently
    // have a set of routines to refresh the certificates, make it
    // REALLY long.
    X509_gmtime_adj(X509_get_notAfter(cert), seconds);
}


void WvX509::set_key_usage(WvStringParm values)
{
    set_extension(NID_key_usage, values);
}


WvString WvX509::get_key_usage() const
{
    return get_extension(NID_key_usage);
}


void WvX509::set_ext_key_usage(WvStringParm values)
{
    set_extension(NID_ext_key_usage, values);
}


WvString WvX509::get_ext_key_usage() const
{
    return get_extension(NID_ext_key_usage);
}


WvString WvX509::get_altsubject() const
{
    return get_extension(NID_subject_alt_name);
}


bool WvX509::get_basic_constraints(bool &ca, int &pathlen) const
{
    CHECK_CERT_EXISTS_GET("basic constraints", false);
    
    BASIC_CONSTRAINTS *constraints = NULL;
    int i;

    constraints = static_cast<BASIC_CONSTRAINTS *>
	(X509_get_ext_d2i(cert, NID_basic_constraints, &i, NULL));
    if (constraints)
    {
        ca = constraints->ca;
        if (constraints->pathlen)
        {
            if ((constraints->pathlen->type == V_ASN1_NEG_INTEGER) || !ca)
            {
                debug("Path length type not valid when getting basic "
                      "constraints.\n");
                BASIC_CONSTRAINTS_free(constraints);
                pathlen = 0;
                return false;
            }
            
            pathlen = ASN1_INTEGER_get(constraints->pathlen);
        }
        else
            pathlen = (-1);

        BASIC_CONSTRAINTS_free(constraints);
        return true;
    }
    
    debug("Basic constraints extension not present.\n");
    return false;
}


void WvX509::set_basic_constraints(bool ca, int pathlen)
{
    CHECK_CERT_EXISTS_SET("basic constraints");

    BASIC_CONSTRAINTS *constraints = BASIC_CONSTRAINTS_new();
    
    constraints->ca = static_cast<int>(ca);
    if (pathlen != (-1))
    {
        ASN1_INTEGER *i = ASN1_INTEGER_new();
        ASN1_INTEGER_set(i, pathlen);
        constraints->pathlen = i;
    }

    X509_EXTENSION *ex = X509V3_EXT_i2d(NID_basic_constraints, 0, 
                                        constraints);
    while (int idx = X509_get_ext_by_NID(cert, NID_basic_constraints, 0) >= 0)
    {
        debug("Found extension at idx %s\n", idx);
        X509_EXTENSION *tmpex = X509_delete_ext(cert, idx);
        X509_EXTENSION_free(tmpex);
    }

    X509_add_ext(cert, ex, NID_basic_constraints);
    X509_EXTENSION_free(ex);
    BASIC_CONSTRAINTS_free(constraints);
}


/*
 * These functions are optional to the API.  If OpenSSL doesn't support them,
 * we simply won't include them here, and apps that need them won't compile.
 */
#ifdef HAVE_OPENSSL_POLICY_MAPPING

bool WvX509::get_policy_constraints(int &require_explicit_policy, 
                                    int &inhibit_policy_mapping) const
{
    CHECK_CERT_EXISTS_GET("policy constraints", false);

    POLICY_CONSTRAINTS *constraints = NULL;
    int i;
    
    constraints = static_cast<POLICY_CONSTRAINTS *>(X509_get_ext_d2i(
                                                cert, NID_policy_constraints, 
                                                &i, NULL));
    if (constraints)
    {
        if (constraints->requireExplicitPolicy)
            require_explicit_policy = ASN1_INTEGER_get(
                constraints->requireExplicitPolicy);
        else
            require_explicit_policy = (-1);

        if (constraints->inhibitPolicyMapping)
            inhibit_policy_mapping = ASN1_INTEGER_get(
                constraints->inhibitPolicyMapping);
        else
            inhibit_policy_mapping = (-1);
        POLICY_CONSTRAINTS_free(constraints);
        return true;
    }

    return false;
}


void WvX509::set_policy_constraints(int require_explicit_policy, 
                                       int inhibit_policy_mapping)
{
    CHECK_CERT_EXISTS_SET("policy constraints");

    POLICY_CONSTRAINTS *constraints = POLICY_CONSTRAINTS_new();
    
    ASN1_INTEGER *i = ASN1_INTEGER_new();
    ASN1_INTEGER_set(i, require_explicit_policy);
    constraints->requireExplicitPolicy = i;
    i = ASN1_INTEGER_new();
    ASN1_INTEGER_set(i, inhibit_policy_mapping);
    constraints->inhibitPolicyMapping = i;

    X509_EXTENSION *ex = X509V3_EXT_i2d(NID_policy_constraints, 0, 
                                        constraints);
    X509_add_ext(cert, ex, -1);
    X509_EXTENSION_free(ex);
    POLICY_CONSTRAINTS_free(constraints);
}


bool WvX509::get_policy_mapping(PolicyMapList &list) const
{
    CHECK_CERT_EXISTS_GET("policy mapping", false);

    POLICY_MAPPINGS *mappings = NULL;
    POLICY_MAPPING *map = NULL;
    int i;

    mappings = static_cast<POLICY_MAPPINGS *>(X509_get_ext_d2i(
                                                cert, NID_policy_mappings, 
                                                &i, NULL));
    if (!mappings)
        return false;

    const int POLICYID_MAXLEN = 80;
    char tmp1[80];
    char tmp2[80];
    for(int j = 0; j < sk_POLICY_MAPPING_num(mappings); j++) 
    {
        map = sk_POLICY_MAPPING_value(mappings, j);
        OBJ_obj2txt(tmp1, POLICYID_MAXLEN, map->issuerDomainPolicy, true);
        OBJ_obj2txt(tmp2, POLICYID_MAXLEN, map->subjectDomainPolicy, true);
        list.append(new PolicyMap(tmp1, tmp2), true);
    }

    sk_POLICY_MAPPING_pop_free(mappings, POLICY_MAPPING_free);
    
    return true;
}


void WvX509::set_policy_mapping(PolicyMapList &list)
{
    CHECK_CERT_EXISTS_SET("policy mapping");

    POLICY_MAPPINGS *maps = sk_POLICY_MAPPING_new_null();
    
    PolicyMapList::Iter i(list);
    for (i.rewind(); i.next();)
    {
        POLICY_MAPPING *map = POLICY_MAPPING_new();
        map->issuerDomainPolicy = OBJ_txt2obj(i().issuer_domain.cstr(), 0);
        map->subjectDomainPolicy = OBJ_txt2obj(i().subject_domain.cstr(), 0);
        sk_POLICY_MAPPING_push(maps, map);
        printf("Push!\n");
    }

    X509_EXTENSION *ex = X509V3_EXT_i2d(NID_policy_mappings, 0, maps);
    X509_add_ext(cert, ex, -1);
    X509_EXTENSION_free(ex);
    sk_POLICY_MAPPING_pop_free(maps, POLICY_MAPPING_free);
}

#endif // HAVE_OPENSSL_POLICY_MAPPING


static void add_aia(WvStringParm type, WvString identifier,
		    AUTHORITY_INFO_ACCESS *ainfo)
{
    ACCESS_DESCRIPTION *acc = ACCESS_DESCRIPTION_new();
    sk_ACCESS_DESCRIPTION_push(ainfo, acc);
    acc->method = OBJ_txt2obj(type.cstr(), 0);
    acc->location->type = GEN_URI;
    acc->location->d.ia5 = M_ASN1_IA5STRING_new();
    unsigned char *cident 
	= reinterpret_cast<unsigned char *>(identifier.edit());
    ASN1_STRING_set(acc->location->d.ia5, cident, identifier.len());
}


void WvX509::set_aia(WvStringList &ca_urls,
                     WvStringList &responders)
{
    CHECK_CERT_EXISTS_SET("aia");

    AUTHORITY_INFO_ACCESS *ainfo = sk_ACCESS_DESCRIPTION_new_null();

    WvStringList::Iter i(ca_urls);
    for (i.rewind(); i.next();)
        add_aia("caIssuers", i(), ainfo);

    WvStringList::Iter j(responders);
    for (j.rewind(); j.next();)
        add_aia("OCSP", j(), ainfo);

    X509_EXTENSION *ex = X509V3_EXT_i2d(NID_info_access, 0, ainfo);
    X509_add_ext(cert, ex, -1);
    X509_EXTENSION_free(ex);
    sk_ACCESS_DESCRIPTION_pop_free(ainfo, ACCESS_DESCRIPTION_free);
}


WvString WvX509::get_aia() const
{
    return get_extension(NID_info_access);
}


static void parse_stack(WvStringParm ext, WvStringList &list,
			WvStringParm prefix)
{
    WvStringList stack;
    stack.split(ext, ";\n");
    WvStringList::Iter i(stack);
    for (i.rewind();i.next();)
    {
        WvString stack_entry(*i);
        if (strstr(stack_entry, prefix))
        {
            WvString uri(stack_entry.edit() + prefix.len());
            list.append(uri);  
        }
    }
}


void WvX509::get_ocsp(WvStringList &responders) const
{
    parse_stack(get_aia(), responders, "OCSP - URI:");
}


void WvX509::get_ca_urls(WvStringList &urls) const
{
    parse_stack(get_aia(), urls, "CA Issuers - URI:");
}


void WvX509::get_crl_urls(WvStringList &urls) const
{
    parse_stack(get_crl_dp(), urls, "URI:");
}


void WvX509::set_crl_urls(WvStringList &urls)
{
    CHECK_CERT_EXISTS_SET("CRL urls");

    STACK_OF(DIST_POINT) *crldp = sk_DIST_POINT_new_null();
    WvStringList::Iter i(urls);
    for (i.rewind(); i.next();)
    {
        DIST_POINT *point = DIST_POINT_new();
        sk_DIST_POINT_push(crldp, point);

        GENERAL_NAMES *uris = GENERAL_NAMES_new();
        GENERAL_NAME *uri = GENERAL_NAME_new();
        uri->type = GEN_URI;
        uri->d.ia5 = M_ASN1_IA5STRING_new();
        unsigned char *cident
	    = reinterpret_cast<unsigned char *>(i().edit());    
        ASN1_STRING_set(uri->d.ia5, cident, i().len());
        sk_GENERAL_NAME_push(uris, uri);

        point->distpoint = DIST_POINT_NAME_new();
        point->distpoint->name.fullname = uris;
        point->distpoint->type = 0;
    }

    X509_EXTENSION *ex = X509V3_EXT_i2d(NID_crl_distribution_points, 0, crldp);
    X509_add_ext(cert, ex, -1);
    X509_EXTENSION_free(ex);
    sk_DIST_POINT_pop_free(crldp, DIST_POINT_free);
}


bool WvX509::get_policies(WvStringList &policy_oids) const
{
    CHECK_CERT_EXISTS_GET("policies", false);

    int critical;
    CERTIFICATEPOLICIES * policies = static_cast<CERTIFICATEPOLICIES *>(
        X509_get_ext_d2i(cert, NID_certificate_policies, &critical, NULL));
    if (policies)
    {
        for (int i = 0; i < sk_POLICYINFO_num(policies); i++)
        {
            POLICYINFO * policy = sk_POLICYINFO_value(policies, i);
            const int POLICYID_MAXLEN = 80;

            char policyid[POLICYID_MAXLEN];
            OBJ_obj2txt(policyid, POLICYID_MAXLEN, policy->policyid, 
                        true); // don't substitute human-readable names
            policy_oids.append(policyid);
        }

        sk_POLICYINFO_pop_free(policies, POLICYINFO_free);
        return true;
    }

    return false;
}


void WvX509::set_policies(WvStringList &policy_oids)
{
    CHECK_CERT_EXISTS_SET("policies");

    STACK_OF(POLICYINFO) *sk_pinfo = sk_POLICYINFO_new_null();

    WvStringList::Iter i(policy_oids);
    for (i.rewind(); i.next();)
    {
        ASN1_OBJECT *pobj = OBJ_txt2obj(i(), 0);
        POLICYINFO *pol = POLICYINFO_new();
        pol->policyid = pobj;
        sk_POLICYINFO_push(sk_pinfo, pol);
    }

#if 0
    // this code would let you set URL information to a policy
    // qualifier
    POLICYQUALINFO *qual = NULL;
    WvString url(_url);
    if (!!url)
    {
	pol->qualifiers = sk_POLICYQUALINFO_new_null();
	qual = POLICYQUALINFO_new();
	qual->pqualid = OBJ_nid2obj(NID_id_qt_cps);
	qual->d.cpsouri = M_ASN1_IA5STRING_new();
	ASN1_STRING_set(qual->d.cpsuri, url.edit(), url.len());
	sk_POLICYQUALINFO_push(pol->qualifiers, qual);
    }
#endif

    X509_EXTENSION *ex = X509V3_EXT_i2d(NID_certificate_policies, 0, 
					sk_pinfo);
    X509_add_ext(cert, ex, -1);
    X509_EXTENSION_free(ex);
    sk_POLICYINFO_pop_free(sk_pinfo, POLICYINFO_free);
}


WvString WvX509::get_extension(int nid) const
{
    CHECK_CERT_EXISTS_GET("extension", WvString::null);

    WvString retval = WvString::null;
    
    int index = X509_get_ext_by_NID(cert, nid, -1);
    if (index >= 0)
    {
        X509_EXTENSION *ext = X509_get_ext(cert, index);
        
        if (ext)
        {
#if (OPENSSL_VERSION_NUMBER >= 0x10000000L)
            const X509V3_EXT_METHOD *method = X509V3_EXT_get(ext);
#else
            X509V3_EXT_METHOD *method = X509V3_EXT_get(ext);
#endif
            if (!method)
            {
                WvDynBuf buf;
                buf.put(ext->value->data, ext->value->length);
                retval = buf.getstr();
            }
            else
            {
                void *ext_data = NULL;
                // we NEED to use a temporary pointer for ext_value_data,
                // as openssl's ASN1_item_d2i will muck around with it, 
                // even though it's const (at least as of version 0.9.8e). 
                // gah.
#if OPENSSL_VERSION_NUMBER >= 0x0090800fL
                const unsigned char * ext_value_data = ext->value->data;
#else
                unsigned char *ext_value_data = ext->value->data;
#endif
                if (method->it)
                {
                    ext_data = ASN1_item_d2i(NULL, &ext_value_data,
                                             ext->value->length, 
                                             ASN1_ITEM_ptr(method->it));
                    TRACE("Applied generic conversion!\n");
                }
                else
                {
                    ext_data = method->d2i(NULL, &ext_value_data,
                                           ext->value->length);
                    TRACE("Applied method specific conversion!\n");
                }
                
                if (method->i2s)
                {
                    TRACE("String Extension!\n");
                    char *s = method->i2s(method, ext_data); 
                    retval = s;
                    OPENSSL_free(s);
                }
                else if (method->i2v)
                {
                    TRACE("Stack Extension!\n");
                    CONF_VALUE *val = NULL;
                    STACK_OF(CONF_VALUE) *svals = NULL;
                    svals = method->i2v(method, ext_data, NULL);
                    if (!sk_CONF_VALUE_num(svals))
                        retval = "EMPTY";
                    else
                    {
                        WvStringList list;
                        for(int i = 0; i < sk_CONF_VALUE_num(svals); i++)
                        {
                            val = sk_CONF_VALUE_value(svals, i);
                            if (!val->name)
                                list.append(WvString(val->value));
                            else if (!val->value)
                                list.append(WvString(val->name));
                            else 
                            {
                                WvString pair("%s:%s", val->name, val->value);
                                list.append(pair);
                            }
                        }
                        retval = list.join(";\n");
                        }
                    sk_CONF_VALUE_pop_free(svals, X509V3_conf_free);
                }
                else if (method->i2r)
                {
                    TRACE("Raw Extension!\n");
                    WvDynBuf retvalbuf;
                    BIO *bufbio = BIO_new(BIO_s_mem());
                    BUF_MEM *bm;
                    method->i2r(method, ext_data, bufbio, 0);
                    BIO_get_mem_ptr(bufbio, &bm);
                    retvalbuf.put(bm->data, bm->length);
                    BIO_free(bufbio);
                    retval = retvalbuf.getstr();
                }
		    
                if (method->it)
                    ASN1_item_free((ASN1_VALUE *)ext_data, 
                                   ASN1_ITEM_ptr(method->it));
                else
                    method->ext_free(ext_data);

            }
        }
    }
    else
    {
        TRACE("Extension not present!\n");
    }

    if (!!retval)
        TRACE("Returning: %s\n", retval);

    return retval;
}


void WvX509::set_extension(int nid, WvStringParm _values)
{
    CHECK_CERT_EXISTS_SET("extension");

    // first we check to see if the extension already exists, if so we need to
    // kill it
    int index = X509_get_ext_by_NID(cert, nid, -1);
    if (index >= 0)
    {
        X509_EXTENSION *ex = X509_delete_ext(cert, index);
        X509_EXTENSION_free(ex);
    }    

    // now set the extension
    WvString values(_values);
    X509_EXTENSION *ex = NULL;
    ex = X509V3_EXT_conf_nid(NULL, NULL, nid, values.edit());
    X509_add_ext(cert, ex, -1);
    X509_EXTENSION_free(ex);
}


bool WvX509::isok() const
{
    return cert;
}


bool WvX509::operator! () const
{
    return !isok();
}


WvString WvX509::errstr() const
{
    if (!cert)
        return "No certificate.";

    return WvString::empty;
}


bool WvX509::verify(WvStringParm original, WvStringParm signature) const
{
    WvDynBuf buf;
    buf.putstr(original);
    return verify(buf, signature);
}


bool WvX509::verify(WvBuf &original, WvStringParm signature) const
{    
    unsigned char sig_buf[4096];
    size_t sig_size = sizeof(sig_buf);
    WvBase64Decoder().flushstrmem(signature, sig_buf, &sig_size, true);
    
    EVP_PKEY *pk = X509_get_pubkey(cert);
    if (!pk) 
        return false;
    
    /* Verify the signature */
    EVP_MD_CTX sig_ctx;
    EVP_VerifyInit(&sig_ctx, EVP_sha1());
    EVP_VerifyUpdate(&sig_ctx, original.peek(0, original.used()),
		     original.used());
    int sig_err = EVP_VerifyFinal(&sig_ctx, sig_buf, sig_size, pk);
    EVP_PKEY_free(pk);
    EVP_MD_CTX_cleanup(&sig_ctx); // Again, not my fault... 
    if (sig_err != 1) 
    {
        debug("Verify failed!\n");
        return false;
    }
    else
	return true;
}


static time_t ASN1_TIME_to_time_t(ASN1_TIME *t)
{
    struct tm newtime;
    char *p = NULL;
    char d[18];
    memset(&d,'\0',sizeof(d));    
    memset(&newtime,'\0',sizeof newtime);
    
    if (t->type == V_ASN1_GENERALIZEDTIME) 
    {
         // For time values >= 2050, OpenSSL uses
         // ASN1_GENERALIZEDTIME - which we'll worry about
         // later.
	return 0;
    }

    p = (char *)t->data;
    sscanf(p,"%2s%2s%2s%2s%2s%2sZ", d, &d[3], &d[6], &d[9], &d[12], &d[15]);
    
    int year = strtol(d, (char **)NULL, 10);
    if (year < 49)
	year += 100;
    else
	year += 50;
    
    newtime.tm_year = year;
    newtime.tm_mon = strtol(&d[3], (char **)NULL, 10) - 1;
    newtime.tm_mday = strtol(&d[6], (char **)NULL, 10);
    newtime.tm_hour = strtol(&d[9], (char **)NULL, 10);
    newtime.tm_min = strtol(&d[12], (char **)NULL, 10);
    newtime.tm_sec = strtol(&d[15], (char **)NULL, 10);

    return mktime(&newtime);
}


time_t WvX509::get_notvalid_before() const
{
    CHECK_CERT_EXISTS_GET("not valid before", 0);

    return ASN1_TIME_to_time_t(X509_get_notBefore(cert));
}


time_t WvX509::get_notvalid_after() const
{
    CHECK_CERT_EXISTS_GET("not valid after", 0);

    return ASN1_TIME_to_time_t(X509_get_notAfter(cert));
}


WvString WvX509::get_ski() const
{
    CHECK_CERT_EXISTS_GET("ski", WvString::null);

    return get_extension(NID_subject_key_identifier);
}


WvString WvX509::get_aki() const
{
    CHECK_CERT_EXISTS_GET("aki", WvString::null);

    WvStringList aki_list;
    parse_stack(get_extension(NID_authority_key_identifier), aki_list,
		"keyid:");
    if (aki_list.count())
        return aki_list.popstr();

    return WvString::null;
}


WvString WvX509::get_fingerprint(const FprintMode mode) const
{
    CHECK_CERT_EXISTS_GET("fingerprint", WvString::null);
   
    /* Default to SHA-1 because OpenSSL does too */
    const EVP_MD *digest = EVP_sha1();
    if (mode == FingerMD5)
	digest = EVP_md5();

    unsigned char md[EVP_MAX_MD_SIZE];
    unsigned int n;
    if (!X509_digest(cert, digest, md, &n))
    {
	errno = -ENOMEM;
	debug("get_fingerprint: Out of memory\n");
	return WvString::null;
    }

    WvDynBuf store;
    char buf[3];
    unsigned int i = 0;
    do {
	sprintf(buf, "%02X", md[i]);
	store.putstr(buf);
    } while (++i < n && (store.putch(':'), 1));

    return store.getstr();
}


void WvX509::set_ski()
{
    CHECK_CERT_EXISTS_SET("ski");

    ASN1_OCTET_STRING *oct = M_ASN1_OCTET_STRING_new();
    ASN1_BIT_STRING *pk = cert->cert_info->key->public_key;
    unsigned char pkey_dig[EVP_MAX_MD_SIZE];
    unsigned int diglen;

    EVP_Digest(pk->data, pk->length, pkey_dig, &diglen, EVP_sha1(), NULL);

    M_ASN1_OCTET_STRING_set(oct, pkey_dig, diglen);
    X509_EXTENSION *ext = X509V3_EXT_i2d(NID_subject_key_identifier, 0, 
					oct);
    X509_add_ext(cert, ext, -1);
    X509_EXTENSION_free(ext);
    M_ASN1_OCTET_STRING_free(oct);
}


void WvX509::set_aki(const WvX509 &cacert)
{
    CHECK_CERT_EXISTS_SET("aki");

    // can't set a meaningful AKI for subordinate certification without the 
    // parent having an SKI
    ASN1_OCTET_STRING *ikeyid = NULL;
    X509_EXTENSION *ext;
    int i = X509_get_ext_by_NID(cacert.cert, NID_subject_key_identifier, -1);
    if ((i >= 0) && (ext = X509_get_ext(cacert.cert, i)))
        ikeyid = static_cast<ASN1_OCTET_STRING *>(X509V3_EXT_d2i(ext));

    if (!ikeyid)
        return;

    AUTHORITY_KEYID *akeyid = AUTHORITY_KEYID_new();
    akeyid->issuer = NULL;
    akeyid->serial = NULL;
    akeyid->keyid = ikeyid;
    ext = X509V3_EXT_i2d(NID_authority_key_identifier, 0, akeyid);
    X509_add_ext(cert, ext, -1);
    X509_EXTENSION_free(ext); 
    AUTHORITY_KEYID_free(akeyid);
}

