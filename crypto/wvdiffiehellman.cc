/*
 * Worldvisions Weaver Software:
 * Copyright (C) 2003 Net Integration Technologies, Inc.
 *
 * Diffie-Hellman shared secret handshake.
 */

#include <openssl/bn.h>
#include "wvdiffiehellman.h"

WvDiffieHellman::WvDiffieHellman(const unsigned char *_key, int _keylen, 
				 BN_ULONG _generator) :
    generator(_generator)
{
    int problems;
    {
	info = DH_new();
	info->p = BN_bin2bn(_key, _keylen, NULL);
// 	info->p->top = 0;
// 	info->p->dmax = _keylen * 8 / BN_BITS2;
// 	info->p->neg = 0;
// 	info->p->flags = 0;

	info->g = BN_new();
	BN_set_word(info->g, generator);
// 	info->g->d = &generator;
// 	info->g->top = 0;
// 	info->g->dmax = 1;
// 	info->g->neg = 0;
// 	info->p->flags = 0;
    }

    DH_check(info, &problems);
//     if (problems & DH_CHECK_P_NOT_PRIME)
// 	log(WvLog::Error, "Using a composite number for authentication.\n");
//     if (problems & DH_CHECK_P_NOT_SAFE_PRIME)
// 	log(WvLog::Error, "Using a non-safe prime number for authentication.\n");
//     if (problems & DH_NOT_SUITEABLE_GENERATOR)
// 	log(WvLog::Error, "Can you just use 2!!\n");
//     if (problems & DH_UNABLE_TO_CHECK_GENERATOR)
// 	log(WvLog::Notice, "Using a strange argument for diffie-hellman.\n");
    DH_generate_key(info);
}

int WvDiffieHellman::pub_key_len()
{
    return BN_num_bytes(info->pub_key);
}

int WvDiffieHellman::get_public_value(WvBuf &outbuf, int len)
{
    int key_len = BN_num_bytes(info->pub_key);
    if (key_len < len)
	len = key_len;

    // alloca is stack allocated, don't free it.
    unsigned char *foo = (unsigned char*)alloca(key_len);
    BN_bn2bin(info->pub_key, foo);
    outbuf.put(foo, len);

    return len;
}

bool WvDiffieHellman::create_secret(WvBuf &inbuf, size_t in_len, WvBuf& outbuf)
{
    unsigned char *foo = (unsigned char *)alloca(DH_size(info));
    DH_compute_key (foo, BN_bin2bn(inbuf.get(in_len), in_len, NULL), info);

    outbuf.put(foo, DH_size(info));

    return true;
}
