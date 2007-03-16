#include "wvdiriter.h"
#include "wvx509store.h"

WvX509Store::WvX509Store() :
    log("WvX509Store", WvLog::Debug5)
{
    store = NULL;
    lookup = NULL;

    store = X509_STORE_new();
    if (store == NULL)
    {
	seterr("Unable to create certificate store context");
	return;
    }

    lookup = X509_STORE_add_lookup(store, X509_LOOKUP_file());
    if (lookup == NULL)
    {
	seterr("Can't add lookup method");
	return;
    }  
}


WvX509Store::~WvX509Store()
{
    // the lookup is automatically freed by the store
    // X509_LOOKUP_free(lookup);
    X509_STORE_free(store);   
}


void WvX509Store::load(WvStringParm _dir)
{
    // FIXME: This is stupid. We should be using X509_STORE_load_locations
    // instead, only problem is that assumes we're only interested in PEM-encoded
    // files. (work around this by providing a method which allows the user
    // of this class to load files individually)
    WvDirIter d(_dir);
    for (d.rewind(); d.next();)
    {
        if (!X509_LOOKUP_load_file(lookup, d().fullname, X509_FILETYPE_PEM) &&
            !X509_LOOKUP_load_file(lookup, d().fullname, X509_FILETYPE_ASN1))
        {
            log(WvLog::Warning, "Can't add lookup file '%s'.\n", d().fullname);
        }
        else
            log(WvLog::Debug5, "Loaded certificate file '%s' into store.\n", 
                d().fullname);
    }
}


bool WvX509Store::is_signed(WvX509Mgr *cert, WvX509Path *path)
{
    X509_STORE_CTX csc;
    X509_STORE_CTX_init(&csc, store, cert->get_cert(), 
                        path ? path->get_stack() : NULL);
    int result = X509_verify_cert(&csc);
    X509_STORE_CTX_cleanup(&csc);

    return result;
}
