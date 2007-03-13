/* -*- Mode: C++ -*-
 * X.509 certificate store management classes.
 */ 
#ifndef __WVX509STORE_H
#define __WVX509STORE_H

#include "wvx509.h"

#include <openssl/x509_vfy.h>


class WvX509Store : public WvErrorBase
{
  public:
    WvX509Store();
    virtual ~WvX509Store();

    /// Returns true if a certificate is signed by a CA in the store, false 
    /// otherwise
    bool is_signed(WvX509Mgr *cert);
    void load(WvStringParm _dir);

  private:
    X509_STORE *store;
    X509_LOOKUP *lookup;
    
    WvLog log;
};

#endif
