#include "wvtest.h"
#include "wvencoderstream.h"
#include "wvloopback.h"
#include "wvgzip.h"

WVTEST_MAIN("gzip")
{
    //FIXME: this test is associated with bug #6166 
    static const char in_data[] = "a line of text\n";
    
    /* deflated "a compressed line" */
    static const char zin_data[] = {
	0x78, 0x9c, 0x4b, 0x54,
	0x48, 0xce, 0xcf, 0x2d,
	0x28, 0x4a, 0x2d, 0x2e,
	0x4e, 0x4d, 0x51, 0xc8,
	0xc9, 0xcc, 0x4b, 0xe5,
	0x02, 0x00, 0x40, 0x15,
	0x06, 0x89
    };
    
    char *line; 
    
    WvLoopback loopy;
    WvEncoderStream gzip(&loopy);
    gzip.disassociate_on_close = true; 
    
    gzip.write(in_data);
    line = gzip.getline(0);
    WVPASS(line && !strcmp(line, "a line of text"));
    WvGzipEncoder *inflater = new WvGzipEncoder(WvGzipEncoder::Inflate);
    gzip.readchain.append(inflater, true);
    gzip.write(zin_data);
    line = gzip.getline(0);
    WVPASS(line && !strcmp(line, "a compressed line"));
    
    WvLoopback loopy2;
    WvEncoderStream gzip2(&loopy);
    gzip2.disassociate_on_close = true; 
    
    gzip2.write(in_data);
    gzip2.write(zin_data);
    line = gzip2.getline(0);
    WVPASS(line && !strcmp(line, "a line of text"));
    WvGzipEncoder *inflater2 = new WvGzipEncoder(WvGzipEncoder::Inflate);
    gzip2.readchain.append(inflater2, true);
    line = gzip2.getline(0);
//    WVPASS(line && !strcmp(line, "a compressed line"));
    
}

