#include "wvargs.h"
#include "wvcrash.h"
#include "wvfile.h"
#include "wvlog.h"   
#include "wvstrutils.h"
#include "wvx509.h"                        

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
    x509->get_ca_urls(urls);
    wvcon->print("CA Issuers available from:\n%s\n", urls.join("\n"));
    x509->get_ocsp(urls);
    wvcon->print("OCSP Responders available from:\n%s\n", urls.join("\n"));
    x509->get_crl_urls(urls);
    wvcon->print("CRL Distribution Points:\n%s\n", urls.join("\n"));
    wvcon->print("Certificate Policy: %s\n", x509->get_cp_oid());
}


int main(int argc, char **argv)
{
    wvcrash_setup(argv[0]);

    WvString certtype = "pem";
    WvStringList remaining_args;

    WvArgs args;
    args.add_required_arg("certificate");
    args.add_option('t', "type", "Certificate type: der or pem (default: pem)", 
                    "type", certtype);
    if (!args.process(argc, argv, &remaining_args) || remaining_args.count() < 1)
    {
        args.print_help(argc, argv);
        return -1;
    }

    WvX509Mgr x509;
    if (certtype == "der")
        x509.load(WvX509Mgr::CertDER, remaining_args.popstr());   
    else if (certtype == "pem")
        x509.load(WvX509Mgr::CertPEM, remaining_args.popstr());
    else
    {
        wverr->print("Invalid certificate type '%s'\n", certtype);
        return -1;
    }

    if (x509.isok())
        print_details(&x509);
    else
        wverr->print("X509 certificate not valid\n");
    
    return 0;
}
