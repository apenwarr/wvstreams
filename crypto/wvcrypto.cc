/*
 * Worldvisions Tunnel Vision Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Streams with built-in cryptography on read/write.  See wvcrypto.h.
 */
#include "wvcrypto.h"
#include "wvsslhacks.h"
#include "strutils.h"
#include <assert.h>
#include <rand.h>
#include <blowfish.h>
#include <rsa.h>
#include <evp.h>
#include <pem.h>


////////////////////////////// WvRSAKey

void WvRSAKey::init(const char *_keystr, bool priv)
{
    errnum = 0;
    rsa = NULL;
    pub = prv = NULL;
    
    if (!_keystr)
    {
	seterr("RSA keystring is null!");
	return;
    }

    // the ssl library segfaults if the buffer isn't big enough and our key
    // is unexpectedly short... sigh.  There's probably a security hole
    // somewhere in the fact that an invalid key can segfault the library.
    int hexbytes = strlen(_keystr);
    int bufsize = ((hexbytes < 2048) ? 2048 : hexbytes) + 16;
    //int bufsize = hexbytes/2;
    
    unsigned char *keybuf = new unsigned char[bufsize];
    const unsigned char *bufp;
    char *keystr;
    RSA *rp;
    
    keystr = strdup(_keystr);
    
    memset(keybuf, 0, bufsize);
    unhexify(keybuf, keystr);
    bufp = keybuf;
    rp = rsa = RSA_new();
    
    if (priv)
    {
	rsa = wv_d2i_RSAPrivateKey(&rp, &bufp, hexbytes/2);

	if (!rsa)
	{
	    seterr("RSA Key is invalid!");
	    free(keystr);
	}
	else
	{
	    prv = keystr;
	
	    size_t size;
	    unsigned char *iend = keybuf;
	    size = i2d_RSAPublicKey(rsa, &iend);
	    pub = (char *)malloc(size * 2 + 1);
	    ::hexify(pub, keybuf, size);
	}
    }
    else
    {
	rsa = wv_d2i_RSAPublicKey(&rp, &bufp, hexbytes/2);
	if (!rsa)
	{
	    seterr("RSA Key is invalid!");
	    free(keystr);
	}
	else
	{
	    prv = NULL;
	    pub = keystr;
	}
    }
    
    delete[] keybuf;
}


WvRSAKey::WvRSAKey(const WvRSAKey &k)
{
    if (k.prv)
	init(k.private_str(), true);
    else
	init(k.public_str(), false);
}


WvRSAKey::WvRSAKey(const char *_keystr, bool priv)
{
    init(_keystr, priv);
}


WvRSAKey::WvRSAKey(int bits)
{
    rsa = RSA_generate_key(bits, 0x10001, NULL, NULL);
    
    hexify(rsa);
}


WvRSAKey::~WvRSAKey()
{
    if (prv)
	free(prv);
    if (pub)
	free(pub);
    if (rsa)
	RSA_free(rsa);
}


void WvRSAKey::pem2hex(WvStringParm filename)
{
    RSA *rsa = NULL;
    FILE *fp;

    fp = fopen(filename, "r");

    if (!fp)
    {
	seterr("Unable to open %s!",filename);
	return;
    }

    rsa = PEM_read_RSAPrivateKey(fp,NULL,NULL,NULL);

    fclose(fp);

    if (!rsa)
    {
	seterr("Unable to decode PEM File!");
	return;
    }
    else
    {
	hexify(rsa);
	return;
    }
}


void WvRSAKey::hexify(RSA *rsa)
{
    size_t size;
    unsigned char *keybuf, *iend;

    size = i2d_RSAPrivateKey(rsa, NULL);
    iend = keybuf = new unsigned char[size];
    i2d_RSAPrivateKey(rsa, &iend);
    
    prv = (char *)malloc(size * 2 + 1);
    ::hexify(prv, keybuf, size);
    
    iend = keybuf;
    size = i2d_RSAPublicKey(rsa, &iend);
    
    pub = (char *)malloc(size * 2 + 1);
    ::hexify(pub, keybuf, size);

    delete[] keybuf;
}


////////////////////////////// WvMD5

WvMD5::WvMD5(WvStringParm string_or_filename, bool isfile)
{
    MD5_CTX ctx;
    unsigned char temp[20];

    if (isfile)
    {
	unsigned char buf[1024];
	int n;
	FILE *file_to_hash;
	
	file_to_hash = fopen(string_or_filename,"r");
	
	if (file_to_hash != NULL)
	{
	    MD5_Init(&ctx);
	    while ((n = fread(buf, 1, sizeof(buf), file_to_hash)) > 0)
		MD5_Update(&ctx, buf, n);
	    MD5_Final(temp, &ctx);
	    if (ferror(file_to_hash))
		md5_hash_value = NULL;
	    else
	    {
		md5_hash_value = (unsigned char *)calloc(1,sizeof(temp));
		memcpy(md5_hash_value,temp,sizeof(temp));
	    }

	    fclose(file_to_hash);
	}
	else
	{
	    md5_hash_value = NULL;
	}
    }
    else
    {
	MD5_Init(&ctx);
	MD5_Update(&ctx,(const unsigned char *)string_or_filename.cstr(),
		   strlen(string_or_filename));
	MD5_Final(temp, &ctx);
	md5_hash_value = (unsigned char *)calloc(1,sizeof(temp));
	memcpy(md5_hash_value,temp,sizeof(temp));
    }
}


WvMD5::~WvMD5()
{
    free(md5_hash_value);
}


WvString WvMD5::md5_hash() const
{
    int count;
    unsigned char *temp;
    WvString hash_value("");
    
    temp = md5_hash_value;
    for (count = 0; count < 16; count++)
    {
	char buf[3];
	snprintf(buf,3,"%02x", *temp++);
	hash_value.append(buf);
    }
    
    return hash_value;
}

////////////////////////////// WvMessageDigest

WvMessageDigest::WvMessageDigest(WvStringParm string, DigestMode _mode)
{
    mode = _mode;
    init();
    if (!!string)
	EVP_DigestUpdate(mdctx, string, strlen(string));
}

WvMessageDigest::WvMessageDigest(WvStream *s, DigestMode _mode)
{
    mode = _mode;
    init();
    // FIXME: !!!
}

WvMessageDigest::~WvMessageDigest()
{
//    EVP_MD_CTX_cleanup(mdctx);
}

void WvMessageDigest::add(WvStringParm string)
{
    EVP_DigestUpdate(mdctx, string, strlen(string));
}

WvString WvMessageDigest::printable() const
{
    int len, i;
    WvString digest_value("");

//    EVP_DigestFinal_ex(mdctx, raw_digest_value, &len);
    len = 0;

    for(i = 0; i < len; i++) 
    {
	char buf[2];
	snprintf(buf,2,"%02x", raw_digest_value[i]);
	digest_value.append(buf);
    }

    return digest_value;
}

void WvMessageDigest::init()
{
    const EVP_MD *md;

    OpenSSL_add_all_digests();
//    EVP_MD_CTX_init(mdctx);

    switch(mode)
    {
	case MD5:
	    md = EVP_get_digestbyname("MD5");
	case SHA1:
	    md = EVP_get_digestbyname("SHA1");
    }

//    EVP_DigestInit_ex(mdctx, md, NULL);
}

////////////////////////////// WvXOREncoder

WvXOREncoder::WvXOREncoder(const void *_key, size_t _keylen) :
    keylen(_keylen), keyoff(0)
{
    key = new unsigned char[keylen];
    memcpy(key, _key, keylen);
}

WvXOREncoder::~WvXOREncoder()
{
    delete[] key;
}

bool WvXOREncoder::encode(WvBuffer &inbuf, WvBuffer &outbuf, bool flush)
{
    size_t len = inbuf.used();
    unsigned char *data = inbuf.get(len);
    unsigned char *out = outbuf.alloc(len);

    // FIXME: this loop is SLOW! (believe it or not)
    while (len-- > 0)
    {
        *out++ = (*data++) ^ key[keyoff++];
        keyoff %= keylen;
    }
    return true;
}


////////////////////////////// WvRSAEncoder

WvRSAEncoder::WvRSAEncoder(bool _encrypt, const WvRSAKey & _key) :
    encrypt(_encrypt), key(_key)
{
    if (key.isok() && key.rsa != NULL)
        rsasize = RSA_size(key.rsa);
    else
        rsasize = 0; // BAD KEY! (should assert but would break compatibility)
}


WvRSAEncoder::~WvRSAEncoder()
{
}


bool WvRSAEncoder::encode(WvBuffer &in, WvBuffer &out, bool flush)
{
    if (rsasize == 0)
    {
        // IGNORE BAD KEY!
        in.zap();
        return false;
    }
        
    bool success = true;
    if (encrypt)
    {
        // reserve space for PKCS1_PADDING
        const size_t maxchunklen = rsasize - 12;
        while (in.used() != 0)
        {
            size_t chunklen = in.used();
            if (chunklen >= maxchunklen)
                chunklen = maxchunklen;
            else if (! flush)
                break;

            // encrypt a chunk
            unsigned char *data = in.get(chunklen);
            unsigned char *crypt = out.alloc(rsasize);
            size_t cryptlen = RSA_public_encrypt(chunklen, data, crypt,
                key.rsa, RSA_PKCS1_PADDING);
            if (cryptlen != rsasize)
            {
                out.unalloc(rsasize);
                success = false;
            }
        }
    } else {
        const size_t chunklen = rsasize;
        while (in.used() >= chunklen)
        {
            // decrypt a chunk
            unsigned char *crypt = in.get(chunklen);
            unsigned char *data = out.alloc(rsasize);
            int cryptlen = RSA_private_decrypt(chunklen, crypt, data,
                key.rsa, RSA_PKCS1_PADDING);
            if (cryptlen == -1)
            {
                out.unalloc(rsasize);
                success = false;
            }
            else
                out.unalloc(rsasize - cryptlen);
        }
        // flush does not make sense for us here
        if (flush && in.used() != 0)
            success = false;
    }
    return success;
}


////////////////////////////// WvBlowfishEncoder

WvBlowfishEncoder::WvBlowfishEncoder(Mode _mode, bool _encrypt,
    const void *_key, size_t _keysize) :
    mode(_mode), encrypt(_encrypt), key(NULL)
{
    setkey(_key, _keysize);
}


WvBlowfishEncoder::~WvBlowfishEncoder()
{
    delete key;
}


void WvBlowfishEncoder::setkey(const void *_key, size_t _keysize)
{
    delete key;
    key = new BF_KEY;
    keysize = _keysize;
    BF_set_key(key, _keysize, (unsigned char *)_key);
    memset(ivec, 0, sizeof(ivec));
    ivecoff = 0;
}


bool WvBlowfishEncoder::encode(WvBuffer &in, WvBuffer &out, bool flush)
{
    size_t len = in.used();
    bool success = true;
    if (mode == ECB)
    {
        size_t remainder = len & 7;
        len -= remainder;
        if (remainder != 0 && flush)
        {
            if (encrypt)
            {
                // if flushing on encryption, add some randomized padding
                size_t padlen = 8 - remainder;
                unsigned char *pad = in.alloc(padlen);
                RAND_pseudo_bytes(pad, padlen);
                len += 8;
            }
            else // nothing we can do here, flushing does not make sense!
                success = false;
        }
    }
    if (len == 0) return success;
    
    unsigned char *data = in.get(len);
    unsigned char *crypt = out.alloc(len);
    
    switch (mode)
    {
        case ECB:
            // ECB works 64bits at a time
            while (len >= 8)
            {
                BF_ecb_encrypt(data, crypt, key,
                    encrypt ? BF_ENCRYPT : BF_DECRYPT);
                len -= 8;
                data += 8;
                crypt += 8;
            }
            break;

        case CFB:
            // CFB simulates a stream
            BF_cfb64_encrypt(data, crypt, len, key, ivec,
                &ivecoff, encrypt ? BF_ENCRYPT : BF_DECRYPT);
            break;
    }
    return success;
}


////////////////////////////// WvCounterModeEncoder

WvCounterModeEncoder::WvCounterModeEncoder(WvEncoder *_keycrypt,
    const void *_counter, size_t _countersize) :
    keycrypt(_keycrypt), counter(NULL)
{
    setcounter(_counter, _countersize);
}


WvCounterModeEncoder::~WvCounterModeEncoder()
{
    delete keycrypt;
    delete[] counter;
}


void WvCounterModeEncoder::setcounter(const void *_counter, size_t _countersize)
{
    delete counter;
    counter = new unsigned char[_countersize];
    countersize = _countersize;
    memcpy(counter, _counter, countersize);
}


void WvCounterModeEncoder::getcounter(void *_counter) const
{
    memcpy(_counter, counter, countersize);
}


void WvCounterModeEncoder::incrcounter()
{
    for (size_t i = 0; i < countersize && ! ++counter[i]; ++i);
}


bool WvCounterModeEncoder::encode(WvBuffer &in, WvBuffer &out, bool flush)
{
    size_t len = in.used() % countersize;
    if (len == 0) return true;

    while (len >= 0)
    {
        // generate a key stream
        counterbuf.zap();
        counterbuf.put(counter, countersize);
        unsigned char *crypt = out.alloc(0);
        size_t keylen = out.used();
        bool success = keycrypt->encode(counterbuf, out, true);
        keylen = out.used() - keylen;
        if (! success)
        {
            out.unalloc(keylen);
            return false;
        }

        // XOR it with the data
        if (len < keylen)
        {
            out.unalloc(keylen - len);
            keylen = len;
        }
        unsigned char *data = in.get(keylen);
        while (keylen-- > 0)
            *(crypt++) ^= *(data++);
            
        // update the counter
        incrcounter();
    }
    return true;
}


////////////////////////////// WvRSAStream

WvRSAStream::WvRSAStream(WvStream *_cloned,
    const WvRSAKey &_my_private_key, const WvRSAKey &_their_public_key) :
    WvEncoderStream(_cloned)
{
    readchain.append(new WvRSAEncoder(false /*encrypt*/,
        _my_private_key), true);
    writechain.append(new WvRSAEncoder(true /*encrypt*/,
        _their_public_key), true);
    if (_my_private_key.isok() && _my_private_key.rsa)
        min_readsize = RSA_size(_my_private_key.rsa);
}


////////////////////////////// WvBlowfishStream

WvBlowfishStream::WvBlowfishStream(WvStream *_cloned,
    const void *_key, size_t _keysize) :
    WvEncoderStream(_cloned)
{
    readchain.append(new WvBlowfishEncoder(WvBlowfishEncoder::CFB,
        false /*encrypt*/, _key, _keysize), true);
    writechain.append(new WvBlowfishEncoder(WvBlowfishEncoder::CFB,
        true /*encrypt*/, _key, _keysize), true);
}


////////////////////////////// WvXORStream

WvXORStream::WvXORStream(WvStream *_cloned,
    const void *_key, size_t _keysize) :
    WvEncoderStream(_cloned)
{
    readchain.append(new WvXOREncoder(_key, _keysize), true);
    writechain.append(new WvXOREncoder(_key, _keysize), true);
}
