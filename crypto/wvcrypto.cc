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
#include <blowfish.h>
#include <rsa.h>
#include <evp.h>
#include <pem.h>

////////////////////////// WvCryptoStream



WvCryptoStream::WvCryptoStream(WvStream *_slave) : WvStreamClone(_slave)
{
    my_cryptbuf = NULL;
    cryptbuf_size = 0;
}


WvCryptoStream::~WvCryptoStream()
{
    if (my_cryptbuf)
	delete[] my_cryptbuf;

    // Preserve the old WvStreamClone semantics, TunnelVision expects
    // it.
    cloned = NULL;
}


unsigned char *WvCryptoStream::cryptbuf(size_t size)
{
    if (size > cryptbuf_size)
    {
	if (my_cryptbuf)
	    delete[] my_cryptbuf;
	cryptbuf_size = size;
	my_cryptbuf = new unsigned char[size];
    }
    
    return my_cryptbuf;
}


/////////////////////// WvXORStream



WvXORStream::WvXORStream(WvStream *_slave, unsigned char _xorvalue)
		: WvCryptoStream(_slave)
{
    xorvalue = _xorvalue;
}


size_t WvXORStream::uwrite(const void *buf, size_t size)
{
    unsigned char *out = cryptbuf(size);
    const unsigned char *i = (const unsigned char *)buf;
    unsigned char *o = out;
    size_t count;
    
    for (count = 0; count < size; count++)
	*o++ = (*i++) ^ xorvalue;
    
    return WvCryptoStream::uwrite(out, size);
}


size_t WvXORStream::uread(void *buf, size_t size)
{
    unsigned char *in = cryptbuf(size);
    const unsigned char *i = (const unsigned char *)in;
    unsigned char *o = (unsigned char *)buf;
    size_t count;
    
    size = WvCryptoStream::uread(in, size);
    
    for (count = 0; count < size; count++)
	*o++ = (*i++) ^ xorvalue;
    return size;
}



////////////////////////// WvBlowfishStream



WvBlowfishStream::WvBlowfishStream(WvStream *_slave, const void *_key,
				   size_t keysize)
		: WvCryptoStream(_slave)
{
    key = new BF_KEY;
    BF_set_key(key, keysize, (unsigned char *)_key);
    ennum = denum = 0;
    memset(envec, 0, sizeof(envec));
    memset(devec, 0, sizeof(devec));
}


size_t WvBlowfishStream::uwrite(const void *buf, size_t size)
{
    void *out = cryptbuf(size);
    
    BF_cfb64_encrypt((unsigned char *)buf,
		     (unsigned char *)out, size, key,
		     envec, &ennum, BF_ENCRYPT);
    
    return WvCryptoStream::uwrite(out, size);
}


size_t WvBlowfishStream::uread(void *buf, size_t size)
{
    void *in = cryptbuf(size);
    size = WvCryptoStream::uread(in, size);
    
    BF_cfb64_encrypt((unsigned char *)in,
		     (unsigned char *)buf, size, key,
		     devec, &denum, BF_DECRYPT);
    
    return size;
}


WvBlowfishStream::~WvBlowfishStream()
{
    delete key;
}



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
    rsa = RSA_generate_key(bits, 3, NULL, NULL);
    
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


/////////////////////////// WvRSAStream



WvRSAStream::WvRSAStream(WvStream *_slave,
			 WvRSAKey &_my_key, WvRSAKey &_their_key)
		: WvCryptoStream(_slave),
		  my_key(_my_key.private_str(), true),
		  their_key(_their_key.public_str(), false)
{
    // we always want to read encrypted data in multiples of RSA_size.
    if (my_key.rsa)
	cloned->queuemin(RSA_size(my_key.rsa));
}


WvRSAStream::~WvRSAStream()
{
    // remove our strange queuing requirements
    cloned->queuemin(0);
}


// this function has _way_ too many confusing temporary buffers... but RSA
// can only deal with big chunks of data, and wvstreams doesn't expect that.
size_t WvRSAStream::uread(void *buf, size_t size)
{
    unsigned char *in = cryptbuf(size);
    
    if (!my_key.rsa)
    {
	// make sure we read the data, even if we'll discard it
	WvStreamClone::uread(buf, size);
	return 0;
    }
    
    size_t len, decode_len, rsa_size = RSA_size(my_key.rsa);
    
    if (size > rsa_size)
	size = rsa_size;
    
    len = WvStreamClone::uread(in, rsa_size);
    if (len < rsa_size)
    {
	// didn't get a full packet - should never really happen, since
	// we use queuemin()...
	return 0;
    }
    
    // decrypt the data
    unsigned char *decoded = new unsigned char[rsa_size];
    decode_len = RSA_private_decrypt(len, in, decoded,
				     my_key.rsa, RSA_PKCS1_PADDING);
    
    if (decode_len == (size_t)-1)
	return 0; // error in decoding!
    
    // return up to "size" bytes in the main buffer
    if (decode_len < size)
	size = decode_len;
    memcpy(buf, decoded, size);
    
    // save the remainder in inbuf
    inbuf.put(decoded+size, decode_len - size);
    
    delete decoded;
    return size;
}


size_t WvRSAStream::uwrite(const void *buf, size_t size)
{
    if (!their_key.rsa)
    {
	// invalid key; just pretend to write so we don't get stuff stuck
	// in the buffer.
	return size; 
    }
    
    size_t off, len, totalwrite = 0, rsa_size = RSA_size(my_key.rsa), outsz;
    unsigned char *out = cryptbuf(rsa_size);
    
    // break it into blocks of no more than rsa_size each
    for (off = 0; off < size; off += rsa_size/2)
    {
	if (size-off < rsa_size/2)
	    len = size-off;
	else
	    len = rsa_size/2;
	
	outsz = RSA_public_encrypt(len, (unsigned char *)buf+off, out,
				   their_key.rsa, RSA_PKCS1_PADDING);
	assert(outsz == rsa_size);
	
	// FIXME: this isn't really correct.  If uwrite() doesn't manage to
	// write the _entire_ blob at once, we throw out the rest but
	// claim it got written...
	if (WvStreamClone::uwrite(out, outsz))
	    totalwrite += len;
    }
    
    return totalwrite;
}

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
