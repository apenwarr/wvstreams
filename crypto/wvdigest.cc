/*
 * Worldvisions Tunnel Vision Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * MD5, SHA-1 and HMAC digest abstractions.
 */
#include "wvdigest.h"
#include <evp.h>
#include <hmac.h>
#include <assert.h>

/***** WvEVPMDDigest *****/

WvEVPMDDigest::WvEVPMDDigest(env_md_st *_evpmd) :
    evpmd(_evpmd), active(false)
{
    evpctx = new EVP_MD_CTX;
    _reset();
}


WvEVPMDDigest::~WvEVPMDDigest()
{
    cleanup();
    delete evpctx;
}


bool WvEVPMDDigest::_encode(WvBuffer &inbuf, WvBuffer &outbuf,
    bool flush)
{
    size_t len;
    while ((len = inbuf.usedopt()) != 0)
    {
        const unsigned char *data = inbuf.get(len);
        EVP_DigestUpdate(evpctx, data, len);
    }
    return true;
}


bool WvEVPMDDigest::_finish(WvBuffer &outbuf)
{
    assert(active);
    unsigned char digest[EVP_MAX_MD_SIZE];
    size_t size;
    EVP_DigestFinal(evpctx, digest, & size);
    active = false;
    outbuf.put(digest, size);
    return true;
}


bool WvEVPMDDigest::_reset()
{
    cleanup();
    EVP_DigestInit(evpctx, evpmd);
    active = true;
    return true;
}


void WvEVPMDDigest::cleanup()
{
    if (active)
    {
        // discard digest
        unsigned char digest[EVP_MAX_MD_SIZE];
        EVP_DigestFinal(evpctx, digest, NULL);
        active = false;
    }
}

size_t WvEVPMDDigest::digestsize() const
{
    return EVP_MD_size(evpmd);
}


/***** WvMD5Digest *****/

WvMD5Digest::WvMD5Digest() : WvEVPMDDigest(EVP_md5())
{
}


/***** WvSHA1Digest *****/

WvSHA1Digest::WvSHA1Digest() : WvEVPMDDigest(EVP_sha1())
{
}


/***** WvHMACDigest *****/

WvHMACDigest::WvHMACDigest(WvEVPMDDigest *_digest,
    const void *_key, size_t _keysize) :
    digest(_digest), keysize(_keysize), active(false)
{
    key = new unsigned char[keysize];
    memcpy(key, _key, keysize);
    hmacctx = new HMAC_CTX;
    _reset();
}

WvHMACDigest::~WvHMACDigest()
{
    cleanup();
    delete hmacctx;
    delete[] key;
    delete digest;
}


bool WvHMACDigest::_encode(WvBuffer &inbuf, WvBuffer &outbuf,
    bool flush)
{
    size_t len;
    while ((len = inbuf.usedopt()) != 0)
    {
        const unsigned char *data = inbuf.get(len);
        HMAC_Update(hmacctx, data, len);
    }
    return true;
}


bool WvHMACDigest::_finish(WvBuffer &outbuf)
{
    assert(active);
    unsigned char digest[EVP_MAX_MD_SIZE];
    size_t size;
    HMAC_Final(hmacctx, digest, & size);
    active = false;
    outbuf.put(digest, size);
    return true;
}


bool WvHMACDigest::_reset()
{
    cleanup();
    HMAC_Init(hmacctx, key, keysize, digest->getevpmd());
    active = true;
    return true;
}


void WvHMACDigest::cleanup()
{
    if (active)
    {
        // discard digest
        unsigned char digest[EVP_MAX_MD_SIZE];
        HMAC_Final(hmacctx, digest, NULL);
        active = false;
    }
}


size_t WvHMACDigest::digestsize() const
{
    return digest->digestsize();
}
