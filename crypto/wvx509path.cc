/* -*- Mode: C++ -*-
 * X.509 certificate path management classes.
 */ 
#include "wvx509path.h"


WvX509Path::WvX509Path()
{
    assert(x509_stack = sk_X509_new_null()); // not handling OOM
    assert(crl_stack = sk_X509_CRL_new_null()); // not handling OOM
}


WvX509Path::~WvX509Path()
{
    sk_free(x509_stack);
    sk_free(crl_stack);
    x509_list.zap();
    crl_list.zap();
}


void WvX509Path::add_cert(WvX509Mgr *x509)
{
    x509_list.add(x509, true);
    sk_X509_push(x509_stack, x509->get_cert());
}


void WvX509Path::add_crl(WvCRL *crl)
{
    crl_list.add(crl, true);
    sk_X509_CRL_push(crl_stack, crl->getcrl());
}
