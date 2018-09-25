/* -*- Mode: C++ -*-
 * Worldvisions Tunnel Vision Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * RSA cryptography abstractions.
 */
#ifndef __WVRSA_H
#define __WVRSA_H

#include "wverror.h"
#include "wvlog.h"

struct rsa_st;

/**
 * An RSA public key or public/private key pair that can be used for
 * encryption.
 * 
 * Knows how to encode/decode itself into a string of hex digits
 * for easy transport.
 * 
 * @see WvRSAEncoder
 */
class WvRSAKey
{
public:
   /**
    * Type for the @ref encode() and @ref decode() methods.
    * RsaPEM    = PEM Encoded RSA Private Key
    * RsaPubPEM = PEM Encoded RSA Public Key
    * RsaHex    = DER Encoded RSA Private Key in hexified form
    * RsaPubHex = DER Encoded RSA Public Key in hexified form
    */
    enum DumpMode { RsaPEM, RsaPubPEM, RsaHex, RsaPubHex };

    struct rsa_st *rsa;
    
    WvRSAKey();
    WvRSAKey(const WvRSAKey &k);
    WvRSAKey(WvStringParm keystr, bool priv);
    WvRSAKey(struct rsa_st *_rsa, bool priv); // note: takes ownership

    /**
     * Create a new RSA key of bits strength.
     */
    WvRSAKey(int bits);
    
    virtual ~WvRSAKey();
    
    virtual bool isok() const;
    
    /** 
     * Return the information requested by mode. 
     */
    virtual WvString encode(const DumpMode mode) const;
    virtual void encode(const DumpMode mode, WvBuf &buf) const;

    /**
     * Load the information from the format requested by mode into
     * the class - this overwrites the certificate.
     */
    virtual void decode(const DumpMode mode, WvStringParm encoded);
    virtual void decode(const DumpMode mode, WvBuf &encoded);

private:
    bool priv;
    mutable WvLog debug;
};

#endif // __WVRSA_H
