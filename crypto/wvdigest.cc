/*
 * Worldvisions Tunnel Vision Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * MD5, SHA-1 and HMAC digest abstractions.
 */
#include "wvdigest.h"
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <assert.h>
#include <netinet/in.h>
#include <zlib.h>

/***** WvEVPMDDigest *****/

WvEVPMDDigest::WvEVPMDDigest(const env_md_st *_evpmd) :
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


bool WvEVPMDDigest::_encode(WvBuf &inbuf, WvBuf &outbuf,
    bool flush)
{
    size_t len;
    while ((len = inbuf.optgettable()) != 0)
    {
        const unsigned char *data = inbuf.get(len);
        EVP_DigestUpdate(evpctx, data, len);
    }
    return true;
}


bool WvEVPMDDigest::_finish(WvBuf &outbuf)
{
    assert(active);
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int size; // size_t is not an unsigned int on many 64 bit systems
    EVP_DigestFinal(evpctx, digest, & size);
    active = false;
    outbuf.put(digest, size);
    return true;
}


bool WvEVPMDDigest::_reset()
{
    cleanup();
    
    // the typecast is necessary for API compatibility with different
    // versions of openssl.  None of them *actually* change the contents of
    // the pointer.
    EVP_DigestInit(evpctx, (env_md_st *)evpmd);
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
    return EVP_MD_size((env_md_st *)evpmd);
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
    deletev key;
    delete digest;
}


bool WvHMACDigest::_encode(WvBuf &inbuf, WvBuf &outbuf,
    bool flush)
{
    size_t len;
    while ((len = inbuf.optgettable()) != 0)
    {
        const unsigned char *data = inbuf.get(len);
        HMAC_Update(hmacctx, data, len);
    }
    return true;
}


bool WvHMACDigest::_finish(WvBuf &outbuf)
{
    assert(active);
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int size;
    HMAC_Final(hmacctx, digest, & size);
    active = false;
    outbuf.put(digest, size);
    return true;
}


bool WvHMACDigest::_reset()
{
    cleanup();
    HMAC_Init(hmacctx, key, keysize, (env_md_st *)digest->getevpmd());
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


WvCrc32Digest::WvCrc32Digest()
{
    _reset();
}


bool WvCrc32Digest::_encode(WvBuf &inbuf, WvBuf &outbuf, bool flush)
{
    size_t len;
    while ((len = inbuf.optgettable()) != 0)
        crc = crc32(crc, inbuf.get(len), len);
    return true;
}


bool WvCrc32Digest::_finish(WvBuf &outbuf)
{
    unsigned long int crcout = htonl(crc);
    outbuf.put(&crcout, sizeof(crcout));
    return true;
}


bool WvCrc32Digest::_reset()
{
    crc = crc32(0, NULL, 0);
    return true;
}


size_t WvCrc32Digest::digestsize() const
{
    return sizeof(crc);
}


WvAdler32Digest::WvAdler32Digest()
{
    _reset();
}


bool WvAdler32Digest::_encode(WvBuf &inbuf, WvBuf &outbuf, bool flush)
{
    size_t len;
    while ((len = inbuf.optgettable()) != 0)
        crc = adler32(crc, inbuf.get(len), len);
    return true;
}


bool WvAdler32Digest::_finish(WvBuf &outbuf)
{
    unsigned long int crcout = htonl(crc);
    outbuf.put(&crcout, sizeof(crcout));
    return true;
}


bool WvAdler32Digest::_reset()
{
    crc = adler32(0, NULL, 0);
    return true;
}


size_t WvAdler32Digest::digestsize() const
{
    return sizeof(crc);
}
