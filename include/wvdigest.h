/*
 * Worldvisions Tunnel Vision Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * MD5, SHA-1 and HMAC digest abstractions.
 *
 */
#ifndef __WVDIGEST_H
#define __WVDIGEST_H

#include "wvencoder.h"

struct env_md_st;
struct env_md_ctx_st;
struct hmac_ctx_st;

/**
 * Superclass for all message digests.
 * <p>
 * All message digest encoders have the following semantics:
 * <ul>
 * <li>On encode() or flush(), data from the input buffer is
 *     consumed and a message digest function is applied to
 *     incrementally update the internal digest state.
 *     No output is ever generated.</li>
 * <li>On finish(), the message digest is finalized and its value
 *     is written to the output buffer.  Afterwards, no new data
 *     can be processed unless reset() is called.</li>
 * <li>On reset(), the current digest state is discarded allowing
 *     a new stream of data to be processed.</li>
 * </ul></p>
 */
class WvDigest : public WvEncoder
{
public:
    /**
     * Returns the number of bytes in the message digest.
     */
    virtual size_t digestsize() const = 0;
};


/**
 * @internal
 * Base class for all digests constructed using the OpenSSL EVP API.
 */
class WvEVPMDDigest : public WvDigest
{
    friend class WvHMACDigest;
    env_md_st *evpmd;
    env_md_ctx_st *evpctx;
    bool active;

public:
    virtual ~WvEVPMDDigest();
    virtual size_t digestsize() const;

protected:
    WvEVPMDDigest(env_md_st *_evpmd);
    virtual bool _encode(WvBuffer &inbuf, WvBuffer &outbuf,
        bool flush); // consumes input
    virtual bool _finish(WvBuffer &outbuf); // outputs digest
    virtual bool _reset(); // supported: resets digest value
    
    env_md_st *getevpmd()
        { return evpmd; }

private:
    void cleanup();
};


/**
 * MD5 Digest.
 * Has a digest length of 128 bits.
 */
class WvMD5Digest : public WvEVPMDDigest
{
public:
    /**
     * Creates an MD5 digest encoder.
     */
    WvMD5Digest();
    virtual ~WvMD5Digest() { }
};


/**
 * SHA-1 Digest.
 * Has a digest length of 160 bits.
 */
class WvSHA1Digest : public WvEVPMDDigest
{
public:
    /**
     * Creates an MD5 digest encoder.
     */
    WvSHA1Digest();
    virtual ~WvSHA1Digest() { }
};


/**
 * HMAC Message Authentication Code.
 * Has a digest length that equals that of its underlying
 * message digest encoder.
 */
class WvHMACDigest : public WvDigest
{
    WvEVPMDDigest *digest;
    unsigned char *key;
    size_t keysize;
    hmac_ctx_st *hmacctx;
    bool active;

public:
    /**
     * Creates an HMAC digest encoder.
     *
     * @param digest the message digest encoder to use as a
     *        hash function
     * @param key the authentication key
     * @param keysize the key size in bytes
     */
    WvHMACDigest(WvEVPMDDigest *_digest, const void *_key,
        size_t _keysize);
    virtual ~WvHMACDigest();
    virtual size_t digestsize() const;

protected:
    virtual bool _encode(WvBuffer &inbuf, WvBuffer &outbuf,
        bool flush); // consumes input
    virtual bool _finish(WvBuffer &outbuf); // outputs digest
    virtual bool _reset(); // supported: resets digest value

private:
    void cleanup();
};

#endif // __WVDIGEST_H
