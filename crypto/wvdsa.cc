/*
 * Worldvisions Tunnel Vision Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * DSA cryptography abstractions.
 */
#include <assert.h>
#include <openssl/dsa.h>
#include <openssl/pem.h>
#include "wvsslhacks.h"
#include "wvdsa.h"
#include "wvhex.h"

/***** WvDSAKey *****/

WvDSAKey::WvDSAKey(const WvDSAKey &k)
{
    if (k.prv)
	init(k.private_str(), true);
    else
	init(k.public_str(), false);
}


WvDSAKey::WvDSAKey(struct dsa_st *_dsa, bool priv)
{
    if (_dsa == NULL)
    {
        // assert(_dsa);
        pub = WvString::null;
        prv = WvString::null;
        dsa = NULL;
        seterr("Initializing with a NULL key.. are you insane?");
	return;
    }

    dsa = _dsa;
    pub = hexifypub(dsa);
    if (priv)
        prv = hexifyprv(dsa);
}


WvDSAKey::WvDSAKey(WvStringParm keystr, bool priv)
{
    init(keystr, priv);
}


WvDSAKey::WvDSAKey(int bits)
{
    dsa = DSA_generate_parameters(bits, NULL, 0, NULL, NULL, NULL, NULL);
    DSA_generate_key(dsa);
    pub = hexifypub(dsa);
    prv = hexifyprv(dsa);
}


WvDSAKey::~WvDSAKey()
{
    if (dsa)
        DSA_free(dsa);
}


bool WvDSAKey::isok() const
{
   return dsa && !errstring;
}


void WvDSAKey::init(WvStringParm keystr, bool priv)
{
    // Start out with everything nulled out...
    dsa = NULL;
    pub = WvString::null;
    prv = WvString::null;
    
    // unhexify the supplied key
    WvDynBuf keybuf;
    if (!WvHexDecoder().flushstrbuf(keystr,  keybuf, true) ||
	keybuf.used() == 0)
    {
        seterr("DSA key is not a valid hex string");
        return;
    }
    
    size_t keylen = keybuf.used();
    const unsigned char *key = keybuf.get(keylen);
    
    // create the DSA struct
    if (priv)
    {
	dsa = wv_d2i_DSAPrivateKey(NULL, &key, keylen);
        if (dsa != NULL)
        {
            prv = keystr;
            pub = hexifypub(dsa);
        }
    }
    else
    {
	dsa = wv_d2i_DSAPublicKey(NULL, &key, keylen);
        if (dsa != NULL)
        {
            prv = WvString::null;
            pub = keystr;
        }
    }
    if (dsa == NULL)
        seterr("DSA key is invalid");
}



WvString WvDSAKey::getpem(bool privkey)
{
    FILE *fp = tmpfile();
    const EVP_CIPHER *enc;
    
    if (!fp)
    {
	seterr("Unable to open temporary file!");
	return WvString::null;
    }

    if (privkey)
    {
	enc = EVP_get_cipherbyname("dsa");
	PEM_write_DSAPrivateKey(fp, dsa, enc,
			       NULL, 0, NULL, NULL);
    }
    else
    {
	// We should write out the Public Key, which is the DSA Public
	// key, as well as the DH generator information.
//	PEM_write_DSAPublicKey(fp, dsa);
    }
    
    WvDynBuf b;
    size_t len;
    
    rewind(fp);
    while ((len = fread(b.alloc(1024), 1, 1024, fp)) > 0)
	b.unalloc(1024 - len);
    b.unalloc(1024 - len);
    fclose(fp);

    return b.getstr();
}



WvString WvDSAKey::hexifypub(struct dsa_st *dsa)
{
    WvDynBuf keybuf;

    assert(dsa);

    size_t size = i2d_DSAPublicKey(dsa, NULL);
    unsigned char *key = keybuf.alloc(size);
    size_t newsize = i2d_DSAPublicKey(dsa, & key);
    assert(size == newsize);
    assert(keybuf.used() == size);

    return WvString(WvHexEncoder().strflushbuf(keybuf, true));
}


WvString WvDSAKey::hexifyprv(struct dsa_st *dsa)
{
    WvDynBuf keybuf;

    assert(dsa);

    size_t size = i2d_DSAPrivateKey(dsa, NULL);
    unsigned char *key = keybuf.alloc(size);
    size_t newsize = i2d_DSAPrivateKey(dsa, & key);
    assert(size == newsize);

    return WvString(WvHexEncoder().strflushbuf(keybuf, true));
}

