/*
 * Test WvCryptoStream classes by encrypting or decrypting stdin
 * and writing to stdout.
 * 
 * This should let us get an impression of how fast the cryptostreams are, and
 * how much latency they introduce.
 *
 * This test case BADLY needs to be rewritten.
 */
#include "wvcrypto.h"
#include "wvrsa.h"
#include "wvblowfish.h"
#include "wvxor.h"
#include "wvcountermode.h"
#include "wvdigest.h"
#include "wvhex.h"
#include "wvlog.h"
#include "wvtimeutils.h"
#include "wvstreamlist.h"
#include <assert.h>

#define PRIVATE_KEY "3082025b02010002818100b0873b623907cffea3aebca4815e579d06" \
                    "217f8c79f4992776a0efcd4223df678266c03936af92282b0b8233a0" \
                    "0d538a36b8b02800cbb5908e47af45b378091fd51b8c36f78372526d" \
                    "149d621ba2b538d5ad21b5523c36801f6735f504ee068da5821b13f1" \
                    "3ca4966503a9712792a77dfe80cd8b9cc35efc2aae2033605aeb1b02" \
                    "010302818075af7cec260535546d1f286dab943a68aec0ffb2fbf866" \
                    "1a4f15f533816d3f9a56ef2ad0cf1fb6c57207ac226ab38d06cf25ca" \
                    "c555dd23b5b42fca2e77a55b697142efd8b9ce56d244162ed7c16cfc" \
                    "65b77cd0386beac17a77bfb9f1a51db5b64981c6b238032213790ba1" \
                    "f9b76ab3c65d6674d2b9a1c3ef0ba05f855f6cfdeb024100ea833de9" \
                    "b8b6ed75f76c151fc1fe3462944ff5f081dbd5f8c7d17b6c52cb3359" \
                    "2b92921c7a736db8d2a7cc57e86da6885206dfe06b72fcc6efd57398" \
                    "3dd893b1024100c0b3e688281702a10f8741feb781063dae21f3702e" \
                    "803e4fa3f6239e3a7642a30bacdeec22c483c05cca6a22ac04f34c20" \
                    "603e6f1addbc4ea9681d53135eda8b0241009c577e9bd079f3a3fa48" \
                    "0e152bfecd970d8aa3f5abe7e3fb2fe0fcf2e1dccce61d0c616851a2" \
                    "4925e1c532e5459e6f058c04954047a1fdd9f538f7bad3e5b7cb0241" \
                    "008077ef05700f57160a5a2bff2500aed3c96bf7a01f00298a6d4ec2" \
                    "697c4ed7175d1de9f2c1d857d593319c171d58a232c040299f673e7d" \
                    "89c64568e20ce9e70702406fb58e17541b988269c2e739063bfa836f" \
                    "98493c0d43791cf3ee8374e51c6d519ec08194e5c482362126a8b805" \
                    "758ea0ee40f3a36c947bb4d957b51ac56430ab"

extern char *optarg;

static void usage(WvLog &log, const char *progname)
{
    log.lvl(WvLog::Error);
    log("Usage: %s -x|-r|-b [-E|-D]\n"
	"   where -x is for an XOREncoder test\n"
	"         -r is for an RSAEncoder test\n"
	"         -b is for a BlowfishEncoder test\n"
        "         -c is for a CounterModeEncoder Blowfish test\n"
        "         -M is for an MD5Digest test\n"
        "         -S is for a SHA1Digest test\n"
        "         -H is for an HMACDigest over SHA1Digest test\n"
	"         -E encrypts stdin (default)\n"
	"         -D decrypts stdin (makes no sense for digests)\n"
        "         -i XXX sets the input file",
        "         -o XXX sets the output file",
	progname);
}


size_t copy(WvStream *in, WvStream *out)
{
    size_t total = 0;
    char buf[10240];

    WvStreamList slist;
    slist.append(in, false);
    slist.append(out, false);
    while (in->isok() && out->isok())
    {
        if (slist.select(-1))
            slist.callback();
        size_t len = in->read(buf, sizeof(buf));
        if (len != 0)
        {
            total += len;
            out->write(buf, len);
        }
    }
    out->flush(0);
    return total;
}


int main(int argc, char **argv)
{
    WvLog log(argv[0], WvLog::Info);
    int opt;
    enum { None, XOR, RSA, Blowfish, CounterMode,
        MD5, SHA1, HMAC } crypt_type = None;
    enum { Encrypt, Decrypt } direction = Encrypt;
    WvRSAKey *rsakey = NULL;
    unsigned char *blowkey = NULL;
    WvStreamClone *crypto;
    int numbits = 0;
    struct timeval start, stop;
    struct timezone tz;

    WvStream *in = NULL;
    WvStream *out = NULL;
    
    if (argc < 2)
    {
	usage(log, argv[0]);
	return 1;
    }
    
    while ((opt = getopt(argc, argv, "?xrbcMSHEDB:i:o:")) >= 0)
    {
	switch (opt)
	{
	case '?':
	    usage(log, argv[0]);
	    return 2;
	    
	case 'x':
	    crypt_type = XOR;
	    numbits = 8;
	    break;
	    
	case 'r':
	    crypt_type = RSA;
	    if (!numbits) numbits = 1024;
	    break;
	    
	case 'b':
	    crypt_type = Blowfish;
	    if (!numbits) numbits = 128;
	    break;

        case 'c':
            crypt_type = CounterMode;
	    if (!numbits) numbits = 128;
            break;

        case 'M':
            crypt_type = MD5;
            numbits = 128;
            direction = Decrypt;
            break;

        case 'S':
            crypt_type = SHA1;
            numbits = 160;
            direction = Decrypt;
            break;

        case 'H':
            crypt_type = HMAC;
	    if (!numbits) numbits = 128;
            direction = Decrypt;
            break;
	    
	case 'E':
	    direction = Encrypt;
	    break;
	    
	case 'D':
	    direction = Decrypt;
	    break;
	    
	case 'B':
	    numbits = atoi(optarg);
	    break;
            
        case 'i':
            delete in;
            in = new WvFile(optarg, O_RDONLY);
            break;
            
        case 'o':
            delete out;
            out = new WvFile(optarg, O_WRONLY | O_CREAT);
            break;
	}
    }
    if (! in) in = wvin;
    if (! out) out = wvout;
    
    if (crypt_type == None)
    {
	log(WvLog::Error, "No encryption type specified on command line.\n");
	usage(log, argv[0]);
	return 4;
    }
    
    if (!numbits || numbits < 8 || numbits > 4096)
    {
	log(WvLog::Error, "Invalid key size: %s bits.\n", numbits);
	return 5;
    }

    WvStreamClone *base;
    if (direction == Encrypt)
        base = new WvStreamClone(out);
    else
        base = new WvStreamClone(in);
    base->disassociate_on_close = true;
    
    switch (crypt_type)
    {
    case XOR: {
	log("Using 8-bit XOR encryption.\n");
        char key = 1;
	crypto = new WvXORStream(base, &key, 1);
	break;
    }
	
    case RSA:
        numbits = 1024;
	log("Using %s-bit RSA encryption.\n", numbits);
	//log("Generating key...");
        //rsakey = new WvRSAKey(numbits);
	//log("ok.\n");
        rsakey = new WvRSAKey(PRIVATE_KEY, true);
	crypto = new WvRSAStream(base, *rsakey, *rsakey);
	break;
	
    case Blowfish:
        log("Using %s-bit Blowfish encryption.\n", numbits);
        blowkey = new unsigned char[numbits/8];
        for (int count = 0; count < numbits/8; count++)
            blowkey[count] = count;
        
        crypto = new WvBlowfishStream(base, blowkey, numbits/8);
	break;

    case CounterMode:
    {
        log("Using %s-bit Counter Mode Blowfish encryption.\n", numbits);
        unsigned char blowkey[numbits/8];
        for (int count = 0; count < numbits/8; count++)
            blowkey[count] = count;
        
        WvEncoder *enc = new WvCounterModeEncoder(
            new WvBlowfishEncoder(WvBlowfishEncoder::ECBEncrypt,
            blowkey, numbits / 8), "\0\0\0\0\0\0\0\0", 8);
        WvEncoderStream *encstream = new WvEncoderStream(base);
        encstream->writechain.append(enc, true);
        encstream->readchain.append(enc, false);
        encstream->auto_flush(false);
        crypto = encstream;
	break;
    }

    case MD5:
    {
        log("Using MD5 digest.\n");
        WvEncoder *enc = new WvMD5Digest();
        WvEncoder *hex = new WvHexEncoder();
        WvEncoderStream *encstream = new WvEncoderStream(base);
        encstream->writechain.append(enc, true);
        encstream->writechain.append(hex, true);
        encstream->readchain.append(enc, false);
        encstream->readchain.append(hex, false);
        encstream->auto_flush(false);
        crypto = encstream;
        break;
    }
	
    case SHA1:
    {
        log("Using SHA1 digest.\n");
        WvEncoder *enc = new WvSHA1Digest();
        WvEncoder *hex = new WvHexEncoder();
        WvEncoderStream *encstream = new WvEncoderStream(base);
        encstream->writechain.append(enc, true);
        encstream->writechain.append(hex, true);
        encstream->readchain.append(enc, false);
        encstream->readchain.append(hex, false);
        encstream->auto_flush(false);
        crypto = encstream;
        break;
    }
    
    case HMAC:
    {
        unsigned char key[numbits/8];
        for (int count = 0; count < numbits/8; count++)
            key[count] = count;
            
        log("Using HMAC over SHA1 digest.\n");
        WvEncoder *enc = new WvHMACDigest(new WvSHA1Digest(),
            key, numbits / 8);
        WvEncoder *hex = new WvHexEncoder();
        WvEncoderStream *encstream = new WvEncoderStream(base);
        encstream->writechain.append(enc, true);
        encstream->writechain.append(hex, true);
        encstream->readchain.append(enc, false);
        encstream->readchain.append(hex, false);
        encstream->auto_flush(false);
        crypto = encstream;
        break;
    }
    
    default:
	assert(0);
	break;
    }
    
    gettimeofday(&start, &tz);
    
    size_t total;
    if (direction == Encrypt)
    {
	log("Encrypting stdin to stdout.\n");
        total = copy(in, crypto);
    }
    else
    {
	log("Decrypting stdin to stdout.\n");
        total = copy(crypto, out);
    }
    crypto->close();
    delete crypto;
    if (in != base && in != wvin)
        delete in;
    if (out != base && out != wvout)
        delete out;
    
    gettimeofday(&stop, &tz);
    long tdiff = msecdiff(stop, start);
    
    log("Processed %s bytes in %s.%03s seconds.\n"
	"Transfer rate: %s kbytes/sec.\n",
	total, tdiff/1000, tdiff % 1000, (int)(1000.0 * total / tdiff / 1024));
    
    if (rsakey)
        delete rsakey;
    if (blowkey)
	delete[] blowkey;
}
