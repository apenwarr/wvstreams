/*
 * Worldvisions Tunnel Vision Software:
 *   Copyright (C) 1997-2003 Net Integration Technologies, Inc.
 * 
 * TripleDES cryptography abstractions.
 */
#include "wvtripledes.h"
#include <assert.h>
#include <openssl/rand.h>

/***** WvTripleDESEncoder ****/

WvTripleDESEncoder::WvTripleDESEncoder(Mode _mode, const void *_key1, 
				       const void *_key2, const void *_key3) :
    mode(_mode)
{
    setkey(_key1, _key2, _key3);
}


// WvTripleDESEncoder::~WvTripleDESEncoder()
// {
//     delete[] key;
//     delete deskey1;
//     delete deskey2;
//     delete deskey3;
// }


bool WvTripleDESEncoder::_reset()
{
    memset(ivec, 0, sizeof(ivec));
    ivecoff = 0;
    return true;
}


void WvTripleDESEncoder::setkey(const void *_key1, const void *_key2, 
				const void *_key3)
{
    memcpy(key, _key1, DES_KEY_SZ);
    DES_set_key(&key, &deskey1);

    memcpy(key, _key2, DES_KEY_SZ);
    DES_set_key(&key, &deskey2);

    memcpy(key, _key3, DES_KEY_SZ);
    DES_set_key(&key, &deskey3);

    memset(ivec, 0, sizeof(ivec));
    ivecoff = 0;
}


void WvTripleDESEncoder::setiv(const void *_iv)
{
    memcpy(ivec, _iv, sizeof(ivec));
    ivecoff = 0;
}

bool WvTripleDESEncoder::_encode(WvBuf &in, WvBuf &out, bool flush)
{
    size_t len = in.used();
    bool success = true;
    switch (mode) {
    case ECBEncrypt:
    case ECBDecrypt:
    case CBCEncrypt: // The caller should ensure the padding is correct or
    case CBCDecrypt: // we do it for them, in probably the wrong way.
    {
	size_t remainder = len & 7; // conviently this is the same as len % 8
	len -= remainder;
	if (remainder != 0 && flush)
	{
	    if (mode == ECBEncrypt || mode == CBCEncrypt)
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

    default:
	break;
    }

    if (len == 0) 
	return success;
    
    const unsigned char *data = in.get(len);
    unsigned char *crypt = out.alloc(len);
    
    switch (mode)
    {
    case ECBEncrypt:
    case ECBDecrypt:
	// ECB works 64bits at a time
	while (len >= 8)
	{
#if OPENSSL_VERSION_NUMBER >= 0x0090705fL \
    && OPENSSL_VERSION_NUMBER < 0x0090800fL
	    DES_ecb3_encrypt(data, crypt,
			     &deskey1, &deskey2, &deskey3,
			     mode == ECBEncrypt ? DES_ENCRYPT : DES_DECRYPT);
#else
	    DES_ecb3_encrypt(reinterpret_cast<const_DES_cblock*>(&data),
			     reinterpret_cast<DES_cblock*>(&crypt),
			     &deskey1, &deskey2, &deskey3,
			     mode == ECBEncrypt ? DES_ENCRYPT : DES_DECRYPT);
#endif
	    len -= 8;
	    data += 8;
	    crypt += 8;
	}
	break;

    case CFBEncrypt:
    case CFBDecrypt:
	// CFB simulates a stream
	DES_ede3_cfb64_encrypt(data, crypt, len, &deskey1, &deskey2, &deskey3,
			       &ivec, &ivecoff,
			       mode == CFBEncrypt ? DES_ENCRYPT : DES_DECRYPT);
        break;
    case CBCEncrypt:
	DES_ede3_cbc_encrypt(data, crypt, len, &deskey1, &deskey2, &deskey3,
			     &ivec, DES_ENCRYPT);
	break;
    case CBCDecrypt:
	DES_ede3_cbc_encrypt(data, crypt, len, &deskey1, &deskey2, &deskey3,
			     &ivec, DES_DECRYPT);
	break;
    }
    return success;
}


/***** WvTripleDESStream *****/

WvTripleDESStream::WvTripleDESStream(WvStream *_cloned, const void *_key1,
				     const void *_key2, const void *_key3, 
				     WvTripleDESEncoder::Mode readmode, 
				     WvTripleDESEncoder::Mode writemode) :
    WvEncoderStream(_cloned)
{
    readchain.append(new WvTripleDESEncoder(readmode,
					    _key1, _key2, _key3), true);
    writechain.append(new WvTripleDESEncoder(writemode,
					     _key1, _key2, _key3), true);
}
