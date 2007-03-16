/* -*- Mode: C++ -*-
 * X.509 certificate path management classes.
 */ 
#include "wvx509path.h"


WvX509Path::WvX509Path()
{
    stack = NULL;
    assert(stack = sk_X509_new_null()); // not handling OOM
}


WvX509Path::~WvX509Path()
{
    sk_free(stack);
    x509_list.zap();
}


void WvX509Path::add(WvX509Mgr *x509)
{
    x509_list.add(x509, true);
    sk_X509_push(stack, x509->get_cert());
}
