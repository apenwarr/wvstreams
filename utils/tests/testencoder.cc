#include "wvgzip.h"
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>


class WvXOR : public WvEncoder
{
    unsigned char *key;
    size_t keylen;
    int off;
    
public:
    WvXOR(const void *_key, size_t _keylen);
    virtual ~WvXOR();
    
    bool encode(WvBuffer &in, WvBuffer &out, bool flush);
};


WvXOR::WvXOR(const void *_key, size_t _keylen)
{
    keylen = _keylen;
    key = new unsigned char[keylen];
    memcpy(key, _key, keylen);
    off = 0;
}


WvXOR::~WvXOR()
{
    delete key;
}


bool WvXOR::encode(WvBuffer &inbuf, WvBuffer &outbuf, bool flush)
{
    size_t len = inbuf.used();
    unsigned char *data = inbuf.get(len);
    unsigned char *out = outbuf.alloc(len);

    while (len > 0)
    {
	*out++ = (*data++) ^ key[off++];
	off %= keylen;
	len--;
    }
    
    return true;
}


extern char *optarg;

void usage(const char *prog)
{
    fprintf(stderr, 
	    "Usage: %s <-z|-Z|-x ##> [-f]\n"
	    "    Encode data from stdin to stdout.\n"
	    "        -z: use libz-style encoder (not gzip compatible)\n"
	    "        -Z: use libz-style decoder\n"
	    "        -x: amazing XOR encryption\n"
	    "        -x: amazing XOR decryption\n"
	    "        -f: flush output stream often\n",
	    prog);
}

int main(int argc, char **argv)
{
    WvEncoder *enc = NULL;
    char buf[2048];
    size_t rlen, wlen;
    enum { NoMode, Gzip, Gunzip, XOR } mode = NoMode;
    bool flush_often = false;
    int c;
    const char *xor_key = NULL;
    
    while ((c = getopt(argc, argv, "zZx:f?")) >= 0)
    {
	switch (c)
	{
	case 'z':
	    mode = Gzip;
	    break;
	    
	case 'Z':
	    mode = Gunzip;
	    break;
	    
	case 'x':
	    mode = XOR;
	    xor_key = optarg;
	    break;
	    
	case 'f':
	    flush_often = true;
	    break;
	    
	case '?':
	default:
	    usage(argv[0]);
	    return 1;
	}
    }
    
    switch (mode)
    {
    case Gzip:
	enc = new WvGzip(WvGzip::Compress);
	break;
    case Gunzip:
	enc = new WvGzip(WvGzip::Decompress);
	break;
    case XOR:
	enc = new WvXOR(xor_key, strlen(xor_key));
	break;
	
    case NoMode:
    default:
	usage(argv[0]);
	return 2;
    }
    
    assert(enc);
    
    WvBuffer inbuf;
    WvBuffer outbuf;
    
    while (enc->isok() && (rlen = read(0, buf, sizeof(buf))) >= 0)
    {
	fprintf(stderr, "[read %d bytes]\n", rlen);
        inbuf.put(buf, rlen);
	
	enc->encode(inbuf, outbuf, (rlen==0 || flush_often));
	
	wlen = outbuf.used();
	write(1, outbuf.get(wlen), wlen);
	fprintf(stderr, "[wrote %d bytes]\n", wlen);
	
	if (!rlen)
	    break;
    }
    
    fprintf(stderr, "exiting...\n");
    
    if (rlen < 0)
	perror("read stdin");
    if (!enc->isok())
	fprintf(stderr, "encoder is not okay!\n");
    
    delete enc;
    
    return 0;
}
