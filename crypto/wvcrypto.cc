/*
 * Worldvisions Tunnel Vision Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Miscellaneous cryptography primitives.
 */
#include "wvcrypto.h"
#include <assert.h>
#include <evp.h>
#include <pem.h>

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
    delete[] counter;
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
