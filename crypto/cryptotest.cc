/*
 * Test WvCryptoStream classes by encrypting or decrypting stdin
 * and writing to stdout.
 * 
 * This should let us get an impression of how fast the cryptostreams are, and
 * how much latency they introduce.
 */
#include "wvcrypto.h"
#include "wvlog.h"
#include <assert.h>

extern char *optarg;

static void usage(WvLog &log, const char *progname)
{
    log.lvl(WvLog::Error);
    log("Usage: %s -x|-r|-b [-E|-D]\n"
	"   where -x is for an XORStream test\n"
	"         -r is for an RSAStream test\n"
	"         -b is for a BlowfishStream test\n"
	"         -E encrypts stdin (default)\n"
	"         -D decrypts stdin\n"
	"         -B sets the number of bits in the encryption key",
	progname);
}


static long msecdiff(struct timeval &a, struct timeval &b)
{
    long secdiff, usecdiff;
    
    secdiff = a.tv_sec - b.tv_sec;
    usecdiff = a.tv_usec - b.tv_usec;
    
    return secdiff*1000 + usecdiff/1000;
}


int main(int argc, char **argv)
{
    WvLog log(argv[0], WvLog::Info);
    int opt;
    enum { None, XOR, RSA, Blowfish } crypt_type = None;
    enum { Encrypt, Decrypt } direction = Encrypt;
    WvRSAKey *rsakey = NULL;
    unsigned char *blowkey = NULL;
    WvStream *crypto;
    int numbits = 0;
    char buf[10240];
    size_t len, total;
    struct timeval start, stop;
    struct timezone tz;
    
    if (argc < 2)
    {
	usage(log, argv[0]);
	return 1;
    }
    
    while ((opt = getopt(argc, argv, "?xrbEDB:")) >= 0)
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
	    
	case 'E':
	    direction = Encrypt;
	    break;
	    
	case 'D':
	    direction = Decrypt;
	    break;
	    
	case 'B':
	    numbits = atoi(optarg);
	    break;
	}
    }
    
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
    
    switch (crypt_type)
    {
    case XOR:
	log("Using 8-bit XOR encryption.\n");
	crypto = new WvXORStream(wvcon, 1);
	break;
	
    case RSA:
	log("Using %s-bit RSA encryption.\n", numbits);
	log("Generating key...");
        rsakey = new WvRSAKey(numbits);
	log("ok.\n");
	crypto = new WvRSAStream(wvcon, *rsakey, *rsakey);
	break;
	
    case Blowfish:
	{
	    int count;
	    
	    log("Using %s-bit Blowfish encryption.\n", numbits);
	    blowkey = new unsigned char[numbits/8];
	    for (count = 0; count < numbits/8; count++)
		blowkey[count] = count;
	    
	    crypto = new WvBlowfishStream(wvcon, blowkey, numbits/8);
	}
	break;
	
    default:
	assert(0);
	break;
    }
    
    
    total = 0;
    gettimeofday(&start, &tz);
    
    if (direction == Encrypt)
    {
	log("Encrypting stdin to stdout.\n");
	
	while (wvcon->isok() && crypto->isok())
	{
	    if (wvcon->select(-1))
	    {
		len = wvcon->read(buf, sizeof(buf));
		crypto->write(buf, len);
		total += len;
	    }
	    
	    if (crypto->select(0))
		crypto->callback();
	}
    }
    else // direction == Decrypt
    {
	log("Decrypting stdin to stdout.\n");
	
	while (wvcon->isok() && crypto->isok())
	{
	    if (crypto->select(-1))
	    {
		len = crypto->read(buf, sizeof(buf));
		wvcon->write(buf, len);
		total += len;
	    }
	    
	    if (wvcon->select(0))
		wvcon->callback();
	}
    }
    
    gettimeofday(&stop, &tz);
    long tdiff = msecdiff(stop, start);
    
    
    log("Processed %s bytes in %s.%03s seconds.\n"
	"Transfer rate: %s kbytes/sec.\n",
	total, tdiff/1000, tdiff % 1000, (int)(1000.0 * total / tdiff / 1024));
    
    if (crypto)
	delete crypto;
    if (rsakey)
	delete rsakey;
    if (blowkey)
	delete [] blowkey;
}
