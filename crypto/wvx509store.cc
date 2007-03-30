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
    if (!X509_STORE_load_locations(store, NULL, _dir))
    {
        log(WvLog::Warning, "Couldn't load store from location %s.\n", _dir);
        return;
    }

    log("Loaded store from location %s.\n", _dir);
}


void WvX509Store::add_file(WvStringParm _fname)
{
        if (!X509_LOOKUP_load_file(lookup, _fname, X509_FILETYPE_PEM) &&
            !X509_LOOKUP_load_file(lookup, _fname, X509_FILETYPE_ASN1))
        {
            log(WvLog::Warning, "Can't add lookup file '%s'.\n", _fname);
        }
        else
            log(WvLog::Debug5, "Loaded certificate file '%s' into store.\n", 
                _fname);
}


#if 0
static int callb(int ok, X509_STORE_CTX *ctx)
{
    int err;
    err=X509_STORE_CTX_get_error(ctx);
    if (err == X509_V_ERR_UNHANDLED_CRITICAL_EXTENSION)
        return 1;
    
    WvX509Mgr x(X509_dup(ctx->current_cert));
    fprintf(stderr, "OK: %s (%s)\n", X509_verify_cert_error_string(err), x.get_subject().cstr());
    return ok;
}
#endif

bool WvX509Store::is_signed(WvX509Mgr *cert, WvX509Path *path)
{
    X509_STORE_CTX csc;

    X509_STORE_CTX_init(&csc, store, cert->get_cert(), 
                        path ? path->get_cert_stack() : NULL);

    X509_VERIFY_PARAM *param = X509_STORE_CTX_get0_param(&csc);
    unsigned long flags = X509_VERIFY_PARAM_get_flags(param);
    flags |= X509_V_FLAG_CRL_CHECK;
    flags |= X509_V_FLAG_CRL_CHECK_ALL;
    X509_VERIFY_PARAM_set_flags(param, flags);

    X509_STORE_CTX_set0_crls(&csc, path ? path->get_crl_stack() : NULL);
    //X509_STORE_set_verify_cb_func(&csc, &callb);

    int result = X509_verify_cert(&csc);
    if (!result)
    {
        int err = X509_STORE_CTX_get_error(&csc);
        WvX509Mgr x(X509_dup(csc.current_cert));
        log("Validation failed: %s (failing cert: %s)\n", X509_verify_cert_error_string(err), x.get_subject().cstr());
    }

    X509_STORE_CTX_cleanup(&csc);
    // seems like param is claimed by the store...
    //X509_VERIFY_PARAM_free(param);

    return result;
}
