/*
 * Worldvisions Tunnel Vision Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Miscellaneous cryptography primitives.
 */
#ifndef __WVCRYPTO_H
#define __WVCRYPTO_H

#include "wvfile.h"
#include "wvencoder.h"
#include "wvencoderstream.h"
#include "wvrsa.h"
#include "wvblowfish.h"
#include "wvxor.h"

/**
 * a very simple stream that returns randomness from /dev/urandom
 */
class WvRandomStream : public WvFile
{
public:
    WvRandomStream() : WvFile("/dev/urandom", O_RDONLY) {}
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

#endif // __WVCRYPTO_H
