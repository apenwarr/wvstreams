/* -*- Mode: C++ -*-
 * X.509 certificate path management classes.
 */ 
#ifndef __WVX509PATH_H
#define __WVX509PATH_H
#include "wvcrl.h"
#include "wvlinklist.h"
#include "wvx509.h"

#include <openssl/safestack.h>

DeclareWvList(WvX509Mgr);
DeclareWvList(WvCRL);


class WvX509Path
{
  public:
    WvX509Path();
    virtual ~WvX509Path();
    void add_cert(WvX509Mgr *cert);
    void add_crl(WvCRL *crl);

    STACK_OF(X509) * get_cert_stack() { return x509_stack; }
    STACK_OF(X509_CRL) * get_crl_stack() { return crl_stack; }

  private:
    WvX509MgrList x509_list;
    WvCRLList crl_list;
    STACK_OF(X509) *x509_stack;
    STACK_OF(X509_CRL) *crl_stack;
};

#endif
