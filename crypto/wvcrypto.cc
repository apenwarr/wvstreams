/*
 * Worldvisions Tunnel Vision Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Streams with built-in cryptography on read/write.  See wvcrypto.h.
 */
#include "wvcrypto.h"
#include "strutils.h"
#include <assert.h>
#include <blowfish.h>
#include <rsa.h>



////////////////////////// WvCryptoStream



WvCryptoStream::WvCryptoStream(WvStream *_slave) : WvStreamClone(&slave)
{
    slave = _slave;
    my_cryptbuf = NULL;
    cryptbuf_size = 0;
}


WvCryptoStream::~WvCryptoStream()
{
    if (my_cryptbuf)
	delete[] my_cryptbuf;
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



WvRSAKey::WvRSAKey(char *_keystr, bool priv)
{
    // the ssl library segfaults if the buffer isn't big enough and our key
    // is unexpectedly short... sigh.  There's probably a security hole
    // somewhere in the fact that an invalid key can segfault the library.
    int hexbytes = strlen(_keystr);
    int bufsize = ((hexbytes < 2048) ? 2048 : hexbytes) + 16;
    //int bufsize = hexbytes/2;
    
    unsigned char *keybuf = new unsigned char[bufsize], *bufp;
    char *keystr;
    RSA *rp;
    
    keystr = strdup(_keystr);
    
    memset(keybuf, 0, bufsize);
    unhexify(keybuf, keystr);
    bufp = keybuf;
    rp = rsa = RSA_new();
    
    if (priv)
    {
	rsa = d2i_RSAPrivateKey(&rp, &bufp, hexbytes/2);
	prv = keystr;
	
	size_t size;
	unsigned char *iend = keybuf;
	size = i2d_RSAPublicKey(rsa, &iend);
	pub = (char *)malloc(size * 2 + 1);
	hexify(pub, keybuf, size);
    }
    else
    {
	rsa = d2i_RSAPublicKey(&rp, &bufp, hexbytes/2);
	prv = NULL;
	pub = keystr;
    }
    
    delete[] keybuf;
}


WvRSAKey::WvRSAKey(int bits)
{
    size_t size;
    unsigned char *keybuf, *iend;
    
    rsa = RSA_generate_key(bits, 3, NULL, NULL);
    
    size = i2d_RSAPrivateKey(rsa, NULL);
    iend = keybuf = new unsigned char[size];
    i2d_RSAPrivateKey(rsa, &iend);
    
    prv = (char *)malloc(size * 2 + 1);
    hexify(prv, keybuf, size);
    
    iend = keybuf;
    size = i2d_RSAPublicKey(rsa, &iend);
    
    pub = (char *)malloc(size * 2 + 1);
    hexify(pub, keybuf, size);
    
    delete[] keybuf;
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



/////////////////////////// WvRSAStream



WvRSAStream::WvRSAStream(WvStream *_slave,
			 WvRSAKey &_my_key, WvRSAKey &_their_key)
		: WvCryptoStream(_slave),
		  my_key(_my_key.private_str(), true),
		  their_key(_their_key.public_str(), false)
{
    // we always want to read encrypted data in multiples of RSA_size.
    if (my_key.rsa)
	slave->queuemin(RSA_size(my_key.rsa));
}


WvRSAStream::~WvRSAStream()
{
    // remove our strange queuing requirements
    slave->queuemin(0);
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
