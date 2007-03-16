/* -*- Mode: C++ -*-
 * X.509 certificate path management classes.
 */ 
#ifndef __WVX509PATH_H
#define __WVX509PATH_H
#include "wvlinklist.h"
#include "wvx509.h"

#include <openssl/safestack.h>

DeclareWvList(WvX509Mgr);


class WvX509Path
{
  public:
    WvX509Path();
    virtual ~WvX509Path();
    void add(WvX509Mgr *cert);

    STACK_OF(X509) * get_stack() { return stack; }

  private:
    WvX509MgrList x509_list;
    STACK_OF(X509) *stack;
};


#endif
