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
#include "wvencoder.h"
#include "wvencoderstream.h"

/**
 * a very simple stream that returns randomness from /dev/urandom
 */
class WvRandomStream : public WvFile
{
public:
    WvRandomStream() : WvFile("/dev/urandom", O_RDONLY) {}
};


/**
 * An RSA public key (or public/private key pair) that can be used for
 * encryption.  Knows how to encode/decode itself into a stream of hex
 * digits for easy transport.
 */
struct rsa_st;

class WvRSAKey
{
    int errnum;

    void seterr(WvStringParm s)
            { errstring = s; }
    void seterr(WVSTRING_FORMAT_DECL)
            { seterr(WvString(WVSTRING_FORMAT_CALL)); }    

    void init(const char *_keystr, bool priv);
    void hexify(struct rsa_st *rsa);
        
public:
    struct rsa_st *rsa;
    char *pub, *prv;

    WvRSAKey(const WvRSAKey &k);
    WvRSAKey(const char *_keystr, bool priv);
    WvRSAKey(int bits);
    
    ~WvRSAKey();
    
    char *private_str() const
        { return prv; }
    char *public_str() const
        { return pub; }
        
    void pem2hex(WvStringParm filename);
    
    volatile bool isok() const
        { return (errnum == 0 ? true : false); }
    
    WvString errstring;
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

struct env_md_ctx_st;
/**
 * Message Digest (Cryptographic Hash) of either a string or a File 
 */
class WvMessageDigest
{
public:

   enum DigestMode { MD5 = 0, SHA1 = 1 };
   
   /**
    * Create the _mode Digest of a String 
    */
   WvMessageDigest(WvStringParm string, DigestMode _mode = WvMessageDigest::MD5);   

   /**
    * Create the _mode Digest of a Stream
    */   
   WvMessageDigest(WvStream *s, DigestMode _mode = WvMessageDigest::MD5);

   ~WvMessageDigest();
   
   /**
    * Add a string to the buffer_to_digest
    */
   void add(WvStringParm string);

   /**
    * Sometimes we just want to easily get the text digest for whatever
    * Type of object that we're constructing...
    */
   operator const WvString () const
        { return printable(); }

   WvString printable() const;

private:
   struct env_md_ctx_st *mdctx;
   unsigned char *raw_digest_value;
   WvBuffer buf_to_digest;
   DigestMode mode;

   /**
    * Initialize all of the silly OpenSSL Stuff.
    */
   void init();
};


/**
 * An encoder implementing simple XOR encryption.
 * Mainly useful for testing.
 */
class WvXOREncoder : public WvEncoder
{
public:
    /**
     * Creates a new XOR encoder / decoder.
     *   _key    : the key
     *   _keylen : the length of the key in bytes
     */
    WvXOREncoder(const void *_key, size_t _keylen);
    virtual ~WvXOREncoder();
    
    bool encode(WvBuffer &in, WvBuffer &out, bool flush);

private:
    unsigned char *key;
    size_t keylen;
    int keyoff;
};


/**
 * An encoder implementing RSA public/private key encryption.
 * This is really slow, so should only be used to exchange information
 * about a faster symmetric key (like Blowfish).
 *
 * RSA needs to know the public key from the remote end (to encrypt data) and
 * the private key on this end (to decrypt data).
 */
class WvRSAEncoder : public WvEncoder
{
public:
    /**
     * Creates a new RSA encoder / decoder.
     *   _encrypt : if true, encrypts else decrypts
     *   _key     : the public key for encryption if _encrypt == true,
     *              otherwise the private key for decryption
     */
    WvRSAEncoder(bool _encrypt, const WvRSAKey &_key);
    virtual ~WvRSAEncoder();

    virtual bool encode(WvBuffer &in, WvBuffer &out, bool flush);

private:
    bool encrypt;
    WvRSAKey key;
    size_t rsasize;
};


/**
 * A Blowfish encoder.
 */
struct bf_key_st;
class WvBlowfishEncoder : public WvEncoder
{
public:
    enum Mode {
        ECB, // electronic code book mode (avoid!)
        CFB  // cipher feedback mode (simulates a stream)
    };

    /**
     * Creates a new Blowfish encoder / decoder.
     *   _mode    : the encryption mode
     *   _encrypt : if true, encrypts else decrypts
     *   _key     : the initial key data
     *   _keysize : the initial key size
     */
    WvBlowfishEncoder(Mode _mode, bool _encrypt,
        const void *_key, size_t _keysize);
    virtual ~WvBlowfishEncoder();

    /**
     * Sets the current Blowfish key and resets the initialization
     * vector to all nulls.
     */
    void setkey(const void *_key, size_t _keysize);

    virtual bool encode(WvBuffer &in, WvBuffer &out, bool flush);

private:
    Mode mode;
    bool encrypt;
    size_t keysize;
    struct bf_key_st *key;
    unsigned char ivec[8]; // initialization vector
    int ivecoff; // current offset into initvec
};


/**
 * A counter mode encryption encoder.
 */
class WvCounterModeEncoder
{
public:
    WvEncoder *keycrypt;

    /**
     * Create a new counter mode encoder / decoder.
     *   _keycrypt    : the underlying encoder for generating the keystream
     *                  (note: takes ownership of this encoder)
     *   _counter     : the initial counter value
     *   _countersize : the counter size, must equal crypto block size
     */
    WvCounterModeEncoder(WvEncoder *_keycrypt,
        const void *_counter, size_t _countersize);
    virtual ~WvCounterModeEncoder();

    /**
     * Sets the Counter mode auto-incrementing counter.
     *   counter     : the counter
     *   countersize : the new counter size, must equal crypto block size
     */
    void setcounter(const void *counter, size_t countersize);

    /**
     * Stores the current counter in the supplied buffer.
     *   counter : the array that receives the counter
     */
    void getcounter(void *counter) const;
    
    virtual bool encode(WvBuffer &in, WvBuffer &out, bool flush);
    
private:
    WvBuffer counterbuf;

protected:
    unsigned char *counter; // auto-incrementing counter
    size_t countersize; // counter size in bytes
    
    virtual void incrcounter();    
};


/**
 * A crypto stream implementing RSA public/private key encryption.
 * See WvRSAEncoder for details.
 */
class WvRSAStream : public WvEncoderStream
{
public:
    WvRSAStream(WvStream *_cloned,
        const WvRSAKey &_my_private_key, const WvRSAKey &_their_public_key);
    virtual ~WvRSAStream() { }
};


/**
 * A crypto stream implementing Blowfish CFB encryption.
 * See WvBlowfishEncoder for details.
 */
class WvBlowfishStream : public WvEncoderStream
{
public:
    WvBlowfishStream(WvStream *_cloned, const void *key, size_t _keysize);
    virtual ~WvBlowfishStream() { }
};


/**
 * A crypto stream implementing XOR encryption.
 * See WvXOREncoder for details.
 */
class WvXORStream : public WvEncoderStream
{
public:
    WvXORStream(WvStream *_cloned, const void *key, size_t _keysize);
    virtual ~WvXORStream() { }
};


#endif // __WVCRYPTO_H
