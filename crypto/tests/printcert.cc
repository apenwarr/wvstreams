#include "wvx509.h"                        
#include "wvfile.h"
#include "wvlog.h"   
#include "wvstrutils.h"
#include "wvcrash.h"

#include <openssl/pem.h>   
#include <openssl/x509v3.h>

void print_details(WvX509Mgr *x509)
{
    wvcon->print("Subject: %s\n", x509->get_subject());
    wvcon->print("Issuer: %s\n", x509->get_issuer());
    wvcon->print("Serial: %s\n", x509->get_serial());
    time_t t1 = x509->get_notvalid_before();
    time_t t2 = x509->get_notvalid_after();
    
    wvcon->print("Not Valid Before: %s\n", ctime(&t1));
    wvcon->print("Not Valid After: %s\n", ctime(&t2));
    wvcon->print("Key Usage: %s\n", x509->get_key_usage());
    wvcon->print("Ext Key Usage: %s\n", x509->get_ext_key_usage());
    wvcon->print("Authority Info Access: \n%s\n", x509->get_aia());
    WvStringList urls;
    wvcon->print("CA Issuers available from:\n%s\n", x509->get_ca_urls(&urls)->join("\n"));
    urls.zap();
    wvcon->print("OCSP Responders available from:\n%s\n", x509->get_ocsp(&urls)->join("\n"));
    wvcon->print("CRL Distribution Point:\n%s\n", x509->get_crl_dp());
    wvcon->print("Certificate Policy: %s\n", x509->get_cp_oid());
}



int main(int argc, char **argv)
{
    wvcrash_setup(argv[0]);
    
    WvDynBuf buffer;
    
    if (argc < 2)
    {
	wvcon->print("You must specify a certificate in PEM format\n");
	return -1;
    }
    
    WvFile f(argv[1], O_RDONLY);
    
    while (f.isok())
    {
	f.read(buffer, 1000);
    }
    f.close();
    
    WvX509Mgr x509;
    x509.decode(WvX509Mgr::CertPEM, buffer.getstr());
    
    print_details(&x509);
    
    
    
    return 0;
}
