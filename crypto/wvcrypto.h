/*
 * Worldvisions Tunnel Vision Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Streams with built-in cryptography on read/write.
 */
#ifndef __WVCRYPTO_H
#define __WVCRYPTO_H

#include "wvfile.h"
#include "wvstreamclone.h"

#define WVCRYPTO_BUFSIZE  2048     // max bytes to encrypt at once

/**
 * a very simple stream that returns randomness from /dev/urandom
 */
class WvRandomStream : public WvFile
{
public:
    WvRandomStream() : WvFile("/dev/urandom", O_RDONLY) {}
};


/**
 * base class for other cryptographic streams.  Presumably subclasses will
 * want to redefine uread/uwrite.
 */
class WvCryptoStream : public WvStreamClone
{
    unsigned char *my_cryptbuf;
    size_t cryptbuf_size;
    
protected:
    WvStream *slave;
    unsigned char *cryptbuf(size_t size);
    
public:
    WvCryptoStream(WvStream *_slave);
    virtual ~WvCryptoStream();
};


/**
 * a CryptoStream implementing completely braindead 8-bit XOR encryption.
 * Mainly useful for testing.
 */
class WvXORStream : public WvCryptoStream
{
    unsigned char xorvalue;
public:
    WvXORStream(WvStream *_slave, unsigned char _xorvalue);
    
protected:
    virtual size_t uread(void *buf, size_t size);
    virtual size_t uwrite(const void *buf, size_t size);
};


/**
 * A CryptoStream implementing the fast, symmetric Blowfish encryption via
 * the SSLeay library.  They key is an arbitrary-length sequence of bytes.
 */
struct bf_key_st;

class WvBlowfishStream : public WvCryptoStream
{
    struct bf_key_st *key;
    unsigned char envec[WVCRYPTO_BUFSIZE], devec[WVCRYPTO_BUFSIZE];
    int ennum, denum;
    
public:
    WvBlowfishStream(WvStream *_slave, const void *_key, size_t keysize);
    virtual ~WvBlowfishStream();

protected:
    virtual size_t uread(void *buf, size_t size);
    virtual size_t uwrite(const void *buf, size_t size);
};


/**
 * An RSA public key (or public/private key pair) that can be used for
 * encryption.  Knows how to encode/decode itself into a stream of hex
 * digits for easy transport.
 */
struct rsa_st;

class WvRSAKey
{
    char *pub, *prv;
    int errnum;
    void seterr(WvStringParm _errstring);
    
    void init(const char *_keystr, bool priv);
        
public:
    struct rsa_st *rsa;

    WvRSAKey(const WvRSAKey &k);
    WvRSAKey(const char *_keystr, bool priv);
    WvRSAKey(int bits);
    
    ~WvRSAKey();
    
    char *private_str() const
        { return prv; }
    char *public_str() const
        { return pub; }
        
    volatile bool isok()
    	{ return (errnum == 0 ? true : false); }
    
    WvString errstring;
};


/**
 * A CryptoStream implementing RSA public/private key encryption.  This is
 * really slow, so should only be used to exchange information about a faster
 * symmetric key (like Blowfish).  RSA needs to know the public key from
 * the remote end (to send data) and the private key on this end (to
 * receive data).
 */
class WvRSAStream : public WvCryptoStream
{
    WvRSAKey my_key, their_key;
    size_t decrypt_silly;
    
protected:
    virtual size_t uread(void *buf, size_t size);
    virtual size_t uwrite(const void *buf, size_t size);
    
public:
    WvRSAStream(WvStream *_slave, WvRSAKey &_my_key, WvRSAKey &_their_key);
    virtual ~WvRSAStream();
};

/**
 * MD5 Hash of either a string or a File 
 */
class WvMD5
{
   unsigned char *md5_hash_value;

public:

   /**
    * Create the MD5 Hash of a String or of a File, depending on whether
    * string_or_filename is, well, a string, or a filename.. isfile must be set
    * to false if you want to Hash only a string.
    */
   WvMD5(WvStringParm string_or_filename, bool isfile=true);


   ~WvMD5();
      
   /**
    * MD5 seems to like unsigned char * for some reason, so make it easy
    * to return that type (Probably only be usefull inside other crypto
    * routines, but you never know ;)
    */
   operator const unsigned char *() const
   	{ return md5_hash_value; }

   /**
    * Sometimes we just want to easily get the text MD5 hash for whatever
    * Type of object that we're constructing...
    */
   operator const WvString () const
        { return md5_hash(); }

   WvString md5_hash() const;
};

#endif // __WVCRYPTO_H
