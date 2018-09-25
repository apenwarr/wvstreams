#include "wvtest.h"
#include "wvencoderstream.h"
#include "wvloopback.h"
#include "wvgzip.h"
#include <ctype.h>

#if 1

WVTEST_MAIN("gzip")
{
    //FIXME: this test is associated with bug #6166 
    static const char in_data[] = "a line of text\n";
    
    /* deflated "a compressed line" */
    static const unsigned char zin_data[] = {
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
    WvEncoderStream gzip((loopy.addRef(), &loopy));
    
    gzip.write(in_data);
    line = gzip.blocking_getline(1000);
    WVPASSEQ(line, "a line of text");
    WVFAIL(gzip.isreadable());
    WvGzipEncoder *inflater = new WvGzipEncoder(WvGzipEncoder::Inflate);
    gzip.readchain.append(inflater, true);
    WVFAIL(gzip.isreadable());
    gzip.write(zin_data, sizeof(zin_data));
    line = gzip.blocking_getline(1000);
    WVFAIL(gzip.isreadable());
    WVPASSEQ(line, "a compressed line");
    
    WvLoopback loopy2;
    WvEncoderStream gzip2((loopy.addRef(), &loopy));
    
    gzip2.write(in_data);
    gzip2.write(zin_data, sizeof(zin_data));
    line = gzip2.blocking_getline(1000, '\n', 1);
    WVPASSEQ(line, "a line of text");
    WvGzipEncoder *inflater2 = new WvGzipEncoder(WvGzipEncoder::Inflate);
    gzip2.readchain.append(inflater2, true);
    line = gzip2.blocking_getline(1000);
    WVPASSEQ(line, "a compressed line");
}


#include "wvbufstream.h"
#include "wvbase64.h"

WVTEST_MAIN("Base64")
{
    char input_stuff[] = 
"SnVsIDE0IDE3OjI0OjQ4IEVEVDogV3ZFeGNDb25uPCoxPjogQXBwbHlpbmcgcmVzcG9uc2UgZm9y\n"
"IGNvbW1hbmQgKkp1bCAxNCAxNzoyNDo0OCBFRFQ6IFVwZGF0ZUl0ZW08KjE+OiBSZWFkaW5nOiA2\n"
"MSA2MSA2MSA2MSA2MSA2MSA2MSA2MSA2MSA2MSA2MTEzIDEgSnVsIDE0IDE3OjI0OjQ4IEVEVDog\n"
"VXBkYXRlSXRlbTwqMT46IFJlYWQgNjg1IGJ5dGVzIG9mIGJhc2U2NEp1bCAxNCAxNzoyNDo0OCBF\n"
"RFQ6IFVwZGF0ZUl0ZW08KjE+OiBXcm90ZSA2NzIgYnl0ZXMgaW50byBkZWNvZGVySnVsIDE0IDE3\n"
"OjI0OjQ4IEVEVDogVXBkYXRlSXRlbTwqMT46ICh3cm90ZSBtYXkgYmUgbGVzcyB0aGFuIHJlYWQs\n"
"IGR1ZSB0b25ld2xpbmUgdHJpbW1pbmcpSnVsIDE0IDE3OjI0OjQ4IEVEVDogQ2FsZW5kYXJBZGFw\n"
"dG9yPCoxPjogVG9sZCB0byB1cGRhdGUgaXRlbToxMDg1MDkzNjk1Lk03MzQ1MjZQMTU0MjhSODgw\n"
"NlE3Lm1haUp1bCAxNCAxNzoyNDo0OCBFRFQ6IEV4Y2hhbmdlSXRBZGFwdG9yPCoxPjogQXR0ZW1w\n"
"dGluZyB0byBjb252ZXJ0IGZyb21JU084ODU5LTEgdG8gVVRGLThKdWwgMTQgMTc6MjQ6NDggRURU\n"
"OiBDYWxlbmRhckFkYXB0b3I8KjE+OiBHb3QgYW4gdXBkYXRlIGZvcjEwODUwOTM2OTUuTTczNDUy\n"
"NlAxNTQyOFI4ODA2UTcubWFpSnVsIDE0IDE3OjI0OjQ4IEVEVDogR2VuVXBkYXRlPCoxPjogRmlu\n"
"aXNoZWQgZ2VuZXJpYyBVUERBVEUsIHJldHVybmluZyAxSnVsIDE0IDE3OjI0OjQ4IEVEVDogV3ZF\n"
"eGNDb25uPCoxPjogRm91bmQgMCBuZXdsaW5lcyBhbmQgYXRlIHRoZW1KdWwgMTQgMTc6MjQ6NDgg\n"
"RURUOiBXdkV4Y0Nvbm48KjE+OiA8PCAoKiBVUERBVEUgcGNvbGlqbi5DYWxlbmRhcklQRi5BcHBv\n"
"aW50bWVudCAxMDg1MDkzNjk1Lk0xODQ0NjlQMTU0MjhSNTQyM1ExLm1haSlKdWwgMTQgMTc6MjQ6\n"
"NDggRURUOiBXdkV4Y0Nvbm48KjE+OiBBcHBseWluZyByZXNwb25zZSBmb3IgY29tbWFuZCAqSnVs\n"
"IDE0IDE3OjI0OjQ4IEVEVDogVXBkYXRlSXRlbTwqMT46IFJlYWRpbmc6IDYxIDYxIDYxIDYxIDYx\n"
"IDYxIDYxIDYxIDYxIDYxIDYxMTMgMSBKdWwgMTQgMTc6MjQ6NDggRURUOiBVcGRhdGVJdGVtPCox\n"
"PjogUmVhZCA2ODUgYnl0ZXMgb2YgYmFzZTY0SnVsIDE0IDE3OjI0OjQ4IEVEVDogVXBkYXRlSXRl\n"
"bTwqMT46IFdyb3RlIDY3MiBieXRlcyBpbnRvIGRlY29kZXJKdWwgMTQgMTc6MjQ6NDggRURUOiBV\n"
"cGRhdGVJdGVtPCoxPjogKHdyb3RlIG1heSBiZSBsZXNzIHRoYW4gcmVhZCwgZHVlIHRvbmV3bGlu\n"
"ZSB0cmltbWluZylKdWwgMTQgMTc6MjQ6NDggRURUOiBDYWxlbmRhckFkYXB0b3I8KjE+OiBUb2xk\n"
"IHRvIHVwZGF0ZSBpdGVtOjEwODUwOTM2OTUuTTE4NDQ2OVAxNTQyOFI1NDIzUTEubWFpSnVsIDE0\n"
"IDE3OjI0OjQ4IEVEVDogRXhjaGFuZ2VJdEFkYXB0b3I8KjE+OiBBdHRlbXB0aW5nIHRvIGNvbnZl\n"
"cnQgZnJvbUlTTzg4NTktMSB0byBVVEYtOEp1bCAxNCAxNzoyNDo0OCBFRFQ6IENhbGVuZGFyQWRh\n"
"cHRvcjwqMT46IEdvdCBhbiB1cGRhdGUgZm9yMTA4NTA5MzY5NS5NMTg0NDY5UDE1NDI4UjU0MjNR\n"
"MS5tYWlKdWwgMTQgMTc6MjQ6NDggRURUOiBHZW5VcGRhdGU8KjE+OiBGaW5pc2hlZCBnZW5lcmlj\n"
"IFVQREFURSwgcmV0dXJuaW5nIDFKdWwgMTQgMTc6MjQ6NDggRURUOiBXdkV4Y0Nvbm48KjE+OiBG\n"
"b3VuZCAwIG5ld2xpbmVzIGFuZCBhdGUgdGhlbUp1bCAxNCAxNzoyNDo0OCBFRFQ6IFd2RXhjQ29u\n"
"bjwqMT46IDw8ICgqIFVQREFURSBwY29saWpuLkNhbGVuZGFySVBGLkFwcG9pbnRtZW50IDIwMDQw\n"
"NjI1VDAyNTAzMFotMjczNDgtMTAwLTEtMkBsb3VmYSlKdWwgMTQgMTc6MjQ6NDggRURUOiBXdkV4\n"
"Y0Nvbm48KjE+OiBBcHBseWluZyByZXNwb25zZSBmb3IgY29tbWFuZCAqSnVsIDE0IDE3OjI0OjQ4\n"
"IEVEVDogVXBkYXRlSXRlbTwqMT46IFJlYWRpbmc6IDYxIDYxIDYxIDYxIDYxIDYxIDYxIDYxIDYx\n"
"IDYxIDYxNjEgNjEgNjEgNjEgMzMgMSBKdWwgMTQgMTc6MjQ6NDggRURUOiBVcGRhdGVJdGVtPCox\n"
"PjogUmVhZCA5NDkgYnl0ZXMgb2YgYmFzZTY0SnVsIDE0IDE3OjI0OjQ4IEVEVDogVXBkYXRlSXRl\n"
"bTwqMT46IFdyb3RlIDkzMiBieXRlcyBpbnRvIGRlY29kZXJKdWwgMTQgMTc6MjQ6NDggRURUOiBV\n"
"cGRhdGVJdGVtPCoxPjogKHdyb3RlIG1heSBiZSBsZXNzIHRoYW4gcmVhZCwgZHVlIHRvbmV3bGlu\n"
"ZSB0cmltbWluZylKdWwgMTQgMTc6MjQ6NDggRURUOiBDYWxlbmRhckFkYXB0b3I8KjE+OiBUb2xk\n"
"IHRvIHVwZGF0ZSBpdGVtOjIwMDQwNjI1VDAyNTAzMFotMjczNDgtMTAwLTEtMkBsb3VmYUp1bCAx\n"
"NCAxNzoyNDo0OCBFRFQ6IEV4Y2hhbmdlSXRBZGFwdG9yPCoxPjogQXR0ZW1wdGluZyB0byBjb252\n"
"ZXJ0IGZyb21JU084ODU5LTEgdG8gVVRGLThKdWwgMTQgMTc6MjQ6NDggRURUOiBDYWxlbmRhckFk\n"
"YXB0b3I8KjE+OiBHb3QgYW4gdXBkYXRlIGZvcjIwMDQwNjI1VDAyNTAzMFotMjczNDgtMTAwLTEt\n"
"MkBsb3VmYUp1bCAxNCAxNzoyNDo0OCBFRFQ6IEdlblVwZGF0ZTwqMT46IEZpbmlzaGVkIGdlbmVy\n"
"aWMgVVBEQVRFLCByZXR1cm5pbmcgMUp1bCAxNCAxNzoyNDo0OCBFRFQ6IFd2RXhjQ29ubjwqMT46\n"
"IEZvdW5kIDAgbmV3bGluZXMgYW5kIGF0ZSB0aGVtSnVsIDE0IDE3OjI0OjQ4IEVEVDogV3ZFeGND\n"
"b25uPCoxPjogPDwgKCogVVBEQVRFIHBjb2xpam4uQ2FsZW5kYXJJUEYuQXBwb2ludG1lbnQgMTA4\n"
"ODAzMDY2My5NNzI2MjAzUDIyODhSNjI4MFExLm1haSlKdWwgMTQgMTc6MjQ6NDggRURUOiBXdkV4\n"
"Y0Nvbm48KjE+OiBBcHBseWluZyByZXNwb25zZSBmb3IgY29tbWFuZCAqSnVsIDE0IDE3OjI0OjQ4\n"
"IEVEVDogVXBkYXRlSXRlbTwqMT46IFJlYWRpbmc6IDYxIDYxIDYxIDYxIDYxIDYxIDYxIDYxIDYx\n"
"IDYxIDYxNjEgNjEgNjEgNjEgMjkgMSBKdWwgMTQgMTc6MjQ6NDggRURUOiBVcGRhdGVJdGVtPCox\n"
"PjogUmVhZCA5NDUgYnl0ZXMgb2YgYmFzZTY0SnVsIDE0IDE3OjI0OjQ4IEVEVDogVXBkYXRlSXRl\n"
"bTwqMT46IFdyb3RlIDkyOCBieXRlcyBpbnRvIGRlY29kZXJKdWwgMTQgMTc6MjQ6NDggRURUOiBV\n"
"cGRhdGVJdGVtPCoxPjogKHdyb3RlIG1heSBiZSBsZXNzIHRoYW4gcmVhZCwgZHVlIHRvbmV3bGlu\n"
"ZSB0cmltbWluZylKdWwgMTQgMTc6MjQ6NDggRURUOiBDYWxlbmRhckFkYXB0b3I8KjE+OiBUb2xk\n"
"IHRvIHVwZGF0ZSBpdGVtOjEwODgwMzA2NjMuTTcyNjIwM1AyMjg4UjYyODBRMS5tYWlKdWwgMTQg\n"
"MTc6MjQ6NDggRURUOiBHZW5VcGRhdGU8KjE+OiBGaW5pc2hlZCBnZW5lcmljIFVQREFURSwgcmV0\n"
"dXJuaW5nIDFKdWwgMTQgMTc6MjQ6NDggRURUOiBXdkV4Y0Nvbm48KjE+OiBGb3VuZCAwIG5ld2xp\n"
"bmVzIGFuZCBhdGUgdGhlbUp1bCAxNCAxNzoyNDo0OCBFRFQ6IFd2RXhjQ29ubjwqMT46IDw8ICgq\n"
"IFVQREFURSBwY29saWpuLkNhbGVuZGFySVBGLkFwcG9pbnRtZW50IDIwMDQwNjIzVDA0NTgyM1ot\n"
"MzI1NzctMTAwLTEtOUBsb3VmYSlKdWwgMTQgMTc6MjQ6NDggRURUOiBXdkV4Y0Nvbm48KjE+OiBB\n"
"cHBseWluZyByZXNwb25zZSBmb3IgY29tbWFuZCAqSnVsIDE0IDE3OjI0OjQ4IEVEVDogVXBkYXRl\n"
"SXRlbTwqMT46IFJlYWRpbmc6IDYxIDYxIDYxIDYxIDYxIDYxIDYxIDYxIDYxIDYxIDYxNjEgNjEg\n"
"NjEgNjEgNjEgNjEgNjEgNjEgNjEgNjEgNjEgNjEgNjEgNjEgNjEgNjEgNjEgNjEgNjEgMzcgMSBK\n"
"dWwgMTQgMTc6MjQ6NDggRURUOiBVcGRhdGVJdGVtPCoxPjogUmVhZCAxODY4IGJ5dGVzIG9mIGJh\n"
"c2U2NEp1bCAxNCAxNzoyNDo0OCBFRFQ6IFVwZGF0ZUl0ZW08KjE+OiBXcm90ZSAxODM2IGJ5dGVz\n"
"IGludG8gZGVjb2Rlckp1bCAxNCAxNzoyNDo0OCBFRFQ6IFVwZGF0ZUl0ZW08KjE+OiAod3JvdGUg\n"
"bWF5IGJlIGxlc3MgdGhhbiByZWFkLCBkdWUgdG9uZXdsaW5lIHRyaW1taW5nKUp1bCAxNCAxNzoy\n"
"NDo0OCBFRFQ6IENhbGVuZGFyQWRhcHRvcjwqMT46IFRvbGQgdG8gdXBkYXRlIGl0ZW06MjAwNDA2\n"
"MjNUMDQ1ODIzWi0zMjU3Ny0xMDAtMS05QGxvdWZhSnVsIDE0IDE3OjI0OjQ4IEVEVDogRXhjaGFu\n"
"Z2VJdEFkYXB0b3I8KjE+OiBBdHRlbXB0aW5nIHRvIGNvbnZlcnQgZnJvbUlTTzg4NTktMSB0byBV\n"
"VEYtOEp1bCAxNCAxNzoyNDo0OCBFRFQ6IENhbGVuZGFyQWRhcHRvcjwqMT46IEdvdCBhbiB1cGRh\n"
"dGUgZm9yMjAwNDA2MjNUMDQ1ODIzWi0zMjU3Ny0xMDAtMS05QGxvdWZhSnVsIDE0IDE3OjI0OjQ4\n"
"IEVEVDogR2VuVXBkYXRlPCoxPjogRmluaXNoZWQgZ2VuZXJpYyBVUERBVEUsIHJldHVybmluZyAx\n"
"SnVsIDE0IDE3OjI0OjQ4IEVEVDogV3ZFeGNDb25uPCoxPjogRm91bmQgMCBuZXdsaW5lcyBhbmQg\n"
"YXRlIHRoZW1KdWwgMTQgMTc6MjQ6NDggRURUOiBXdkV4Y0Nvbm48KjE+OiA8PCAoKiBVUERBVEUg\n"
"cGNvbGlqbi5DYWxlbmRhcklQRi5BcHBvaW50bWVudCAxMDg1MDkzNjk1Lk02OTY2NDJQMTU0MjhS\n"
"MjE2OVE2Lm1haSlKdWwgMTQgMTc6MjQ6NDggRURUOiBXdkV4Y0Nvbm48KjE+OiBBcHBseWluZyBy\n"
"ZXNwb25zZSBmb3IgY29tbWFuZCAqSnVsIDE0IDE3OjI0OjQ4IEVEVDogVXBkYXRlSXRlbTwqMT46\n"
"IFJlYWRpbmc6IDYxIDYxIDYxIDYxIDYxIDYxIDYxIDYxIDYxIDYxIDYxMTMgMSBKdWwgMTQgMTc6\n"
"MjQ6NDggRURUOiBVcGRhdGVJdGVtPCoxPjogUmVhZCA2ODUgYnl0ZXMgb2YgYmFzZTY0SnVsIDE0\n"
"IDE3OjI0OjQ4IEVEVDogVXBkYXRlSXRlbTwqMT46IFdyb3RlIDY3MiBieXRlcyBpbnRvIGRlY29k\n"
"ZXJKdWwgMTQgMTc6MjQ6NDggRURUOiBVcGRhdGVJdGVtPCoxPjogKHdyb3RlIG1heSBiZSBsZXNz\n"
"IHRoYW4gcmVhZCwgZHVlIHRvbmV3bGluZSB0cmltbWluZylKdWwgMTQgMTc6MjQ6NDggRURUOiBD\n"
"YWxlbmRhckFkYXB0b3I8KjE+OiBUb2xkIHRvIHVwZGF0ZSBpdGVtOjEwODUwOTM2OTUuTTY5NjY0\n"
"MlAxNTQyOFIyMTY5UTYubWFpSnVsIDE0IDE3OjI0OjQ4IEVEVDogRXhjaGFuZ2VJdEFkYXB0b3I8\n"
"KjE+OiBBdHRlbXB0aW5nIHRvIGNvbnZlcnQgZnJvbUlTTzg4NTktMSB0byBVVEYtOEp1bCAxNCAx\n"
"NzoyNDo0OCBFRFQ6IENhbGVuZGFyQWRhcHRvcjwqMT46IEdvdCBhbiB1cGRhdGUgZm9yMTA4NTA5\n"
"MzY5NS5NNjk2NjQyUDE1NDI4UjIxNjlRNi5tYWlKdWwgMTQgMTc6MjQ6NDggRURUOiBHZW5VcGRh\n"
"dGU8KjE+OiBGaW5pc2hlZCBnZW5lcmljIFVQREFURSwgcmV0dXJuaW5nIDFKdWwgMTQgMTc6MjQ6\n"
"NDggRURUOiBXdkV4Y0Nvbm48KjE+OiBGb3VuZCAwIG5ld2xpbmVzIGFuZCBhdGUgdGhlbUp1bCAx\n"
"NCAxNzoyNDo0OCBFRFQ6IFd2RXhjQ29ubjwqMT46IDw8ICgqIFVQREFURSBwY29saWpuLkNhbGVu\n"
"ZGFySVBGLkFwcG9pbnRtZW50IDEwNzQ4ODM2NzUuTTIyNDQ5N1AzNTg5UjMwNzRRMi5tYWkpSnVs\n"
"IDE0IDE3OjI0OjQ4IEVEVDogV3ZFeGNDb25uPCoxPjogQXBwbHlpbmcgcmVzcG9uc2UgZm9yIGNv\n"
"bW1hbmQgKkp1bCAxNCAxNzoyNDo0OCBFRFQ6IFVwZGF0ZUl0ZW08KjE+OiBSZWFkaW5nOiA2MSA2\n"
"MSA2MSA2MSA2MSA2MSA2MSA2MSA2MSA2MSA2MTEzIDEgSnVsIDE0IDE3OjI0OjQ4IEVEVDogVXBk\n"
"YXRlSXRlbTwqMT46IFJlYWQgNjg1IGJ5dGVzIG9mIGJhc2U2NEp1bCAxNCAxNzoyNDo0OCBFRFQ6\n"
"IFVwZGF0ZUl0ZW08KjE+OiBXcm90ZSA2NzIgYnl0ZXMgaW50byBkZWNvZGVySnVsIDE0IDE3OjI0\n"
"OjQ4IEVEVDogVXBkYXRlSXRlbTwqMT46ICh3cm90ZSBtYXkgYmUgbGVzcyB0aGFuIHJlYWQsIGR1\n"
"ZSB0b25ld2xpbmUgdHJpbW1pbmcpSnVsIDE0IDE3OjI0OjQ4IEVEVDogQ2FsZW5kYXJBZGFwdG9y\n"
"PCoxPjogVG9sZCB0byB1cGRhdGUgaXRlbToxMDc0ODgzNjc1Lk0yMjQ0OTdQMzU4OVIzMDc0UTIu\n"
"bWFpSnVsIDE0IDE3OjI0OjQ4IEVEVDogRXhjaGFuZ2VJdEFkYXB0b3I8KjE+OiBBdHRlbXB0aW5n\n"
"IHRvIGNvbnZlcnQgZnJvbUlTTzg4NTktMSB0byBVVEYtOEp1bCAxNCAxNzoyNDo0OCBFRFQ6IENh\n"
"bGVuZGFyQWRhcHRvcjwqMT46IEdvdCBhbiB1cGRhdGUgZm9yMTA3NDg4MzY3NS5NMjI0NDk3UDM1\n"
"ODlSMzA3NFEyLm1haUp1bCAxNCAxNzoyNDo0OCBFRFQ6IEdlblVwZGF0ZTwqMT46IEZpbmlzaGVk\n"
"IGdlbmVyaWMgVVBEQVRFLCByZXR1cm5pbmcgMUp1bCAxNCAxNzoyNDo0OCBFRFQ6IFd2RXhjQ29u\n"
"bjwqMT46IEZvdW5kIDAgbmV3bGluZXMgYW5kIGF0ZSB0aGVtSnVsIDE0IDE3OjI0OjQ4IEVEVDog\n"
"V3ZFeGNDb25uPCoxPjogPDwgKCogVVBEQVRFIHBjb2xpam4uQ2FsZW5kYXJJUEYuQXBwb2ludG1l\n"
"bnQgMTA4NTA5MzY5Ni5NMzQxMzc3UDE1NDI4UjY2MzZRMTIubWFpKUp1bCAxNCAxNzoyNDo0OCBF\n"
"RFQ6IFd2RXhjQ29ubjwqMT46IEFwcGx5aW5nIHJlc3BvbnNlIGZvciBjb21tYW5kICpKdWwgMTQg\n"
"MTc6MjQ6NDggRURUOiBVcGRhdGVJdGVtPCoxPjogUmVhZGluZzogNjEgNjEgNjEgNjEgNjEgNjEg\n"
"NjEgNjEgNjEgNjEgNjE2MSAxNyAxIEp1bCAxNCAxNzoyNDo0OCBFRFQ6IFVwZGF0ZUl0ZW08KjE+\n"
"OiBSZWFkIDc1MCBieXRlcyBvZiBiYXNlNjRKdWwgMTQgMTc6MjQ6NDggRURUOiBVcGRhdGVJdGVt\n"
"PCoxPjogV3JvdGUgNzM2IGJ5dGVzIGludG8gZGVjb2Rlckp1bCAxNCAxNzoyNDo0OCBFRFQ6IFVw\n"
"ZGF0ZUl0ZW08KjE+OiAod3JvdGUgbWF5IGJlIGxlc3MgdGhhbiByZWFkLCBkdWUgdG9uZXdsaW5l\n"
"IHRyaW1taW5nKUp1bCAxNCAxNzoyNDo0OCBFRFQ6IENhbGVuZGFyQWRhcHRvcjwqMT46IFRvbGQg\n"
"dG8gdXBkYXRlIGl0ZW06MTA4NTA5MzY5Ni5NMzQxMzc3UDE1NDI4UjY2MzZRMTIubWFpSnVsIDE0\n"
"IDE3OjI0OjQ4IEVEVDogRXhjaGFuZ2VJdEFkYXB0b3I8KjE+OiBBdHRlbXB0aW5nIHRvIGNvbnZl\n"
"cnQgZnJvbUlTTzg4NTktMSB0byBVVEYtOEp1bCAxNCAxNzoyNDo0OCBFRFQ6IENhbGVuZGFyQWRh\n"
"cHRvcjwqMT46IEdvdCBhbiB1cGRhdGUgZm9yMTA4NTA5MzY5Ni5NMzQxMzc3UDE1NDI4UjY2MzZR\n"
"MTIubWFpSnVsIDE0IDE3OjI0OjQ4IEVEVDogR2VuVXBkYXRlPCoxPjogRmluaXNoZWQgZ2VuZXJp\n"
"YyBVUERBVEUsIHJldHVybmluZyAxSnVsIDE0IDE3OjI0OjQ4IEVEVDogV3ZFeGNDb25uPCoxPjog\n"
"Rm91bmQgMCBuZXdsaW5lcyBhbmQgYXRlIHRoZW1KdWwgMTQgMTc6MjQ6NDggRURUOiBXdkV4Y0Nv\n"
"bm48KjE+OiA8PCAoKiBMSVNUIHBjb2xpam4uQ2FsZW5kYXJJUEYuQXBwb2ludG1lbnQgMylKdWwg\n"
"MTQgMTc6MjQ6NDggRURUOiBXdkV4Y0Nvbm48KjE+OiBBcHBseWluZyByZXNwb25zZSBmb3IgY29t\n"
"bWFuZCAqSnVsIDE0IDE3OjI0OjQ4IEVEVDogR2VuTGlzdDwqMT46IDMwIGNsaWVudCBpdGVtc0p1\n"
"bCAxNCAxNzoyNDo0OCBFRFQ6IEdlbkxpc3Q8KjE+OiAxMDg1MDkzNjk2Lk0zNDEzNzdQMTU0MjhS\n"
"NjYzNlExMi5tYWkgaXMgaW50aGUgZXZvIHN0b3JlIGFuZCB0aGUgc2VydmVyIHN0b3JlSnVsIDE0\n"
"IDE3OjI0OjQ4IEVEVDogR2VuTGlzdDwqMT46IDEwNzQ4ODM2NzUuTTIyNDQ5N1AzNTg5UjMwNzRR\n"
"Mi5tYWkgaXMgaW4gdGhlZXZvIHN0b3JlIGFuZCB0aGUgc2VydmVyIHN0b3JlSnVsIDE0IDE3OjI0\n"
"OjQ4IEVEVDogR2VuTGlzdDwqMT46IDEwODUwOTM2OTUuTTY5NjY0MlAxNTQyOFIyMTY5UTYubWFp\n"
"IGlzIGludGhlIGV2byBzdG9yZSBhbmQgdGhlIHNlcnZlciBzdG9yZUp1bCAxNCAxNzoyNDo0OCBF\n"
"RFQ6IEdlbkxpc3Q8KjE+OiAyMDA0MDYyM1QwNDU4MjNaLTMyNTc3LTEwMC0xLTlAbG91ZmEgaXMg\n"
"aW50aGUgZXZvIHN0b3JlIGFuZCB0aGUgc2VydmVyIHN0b3JlSnVsIDE0IDE3OjI0OjQ4IEVEVDog\n"
"R2VuTGlzdDwqMT46IDIwMDQwNjI1VDAyNTAzMFotMjczNDgtMTAwLTEtMkBsb3VmYSBpcyBpbnRo\n"
"ZSBldm8gc3RvcmUgYW5kIHRoZSBzZXJ2ZXIgc3RvcmVKdWwgMTQgMTc6MjQ6NDggRURUOiBHZW5M\n"
"aXN0PCoxPjogMTA4NTA5MzY5NS5NMTg0NDY5UDE1NDI4UjU0MjNRMS5tYWkgaXMgaW50aGUgZXZv\n"
"IHN0b3JlIGFuZCB0aGUgc2VydmVyIHN0b3JlSnVsIDE0IDE3OjI0OjQ4IEVEVDogR2VuTGlzdDwq\n"
"MT46IDEwODUwOTM2OTUuTTczNDUyNlAxNTQyOFI4ODA2UTcubWFpIGlzIGludGhlIGV2byBzdG9y\n"
"ZSBhbmQgdGhlIHNlcnZlciBzdG9yZUp1bCAxNCAxNzoyNDo0OCBFRFQ6IEdlbkxpc3Q8KjE+OiAy\n"
"MDA0MDYyM1QwNDAxMTRaLTMyNTc3LTEwMC0xLTBAbG91ZmEgaXMgaW50aGUgZXZvIHN0b3JlIGFu\n"
"ZCB0aGUgc2VydmVyIHN0b3JlSnVsIDE0IDE3OjI0OjQ4IEVEVDogR2VuTGlzdDwqMT46IDEwODcz\n"
"NDA3NDAuTTY4NjIyN1A5NjQ5Ujg3MDJRMC50b3JtZW50IGlzIGludGhlIGV2byBzdG9yZSBhbmQg\n"
"dGhlIHNlcnZlciBzdG9yZUp1bCAxNCAxNzoyNDo0OCBFRFQ6IEdlbkxpc3Q8KjE+OiAyMDA0MDYy\n"
"NVQwNDU3NDNaLTI5Mzc1LTEwMC0xLTBAbG91ZmEgaXMgaW50aGUgZXZvIHN0b3JlIGFuZCB0aGUg\n"
"c2VydmVyIHN0b3JlSnVsIDE0IDE3OjI0OjQ4IEVEVDogR2VuTGlzdDwqMT46IDEwODUwOTM2OTUu\n"
"TTc4NzY3NFAxNTQyOFIzODMxUTgubWFpIGlzIGludGhlIGV2byBzdG9yZSBhbmQgdGhlIHNlcnZl\n"
"ciBzdG9yZUp1bCAxNCAxNzoyNDo0OCBFRFQ6IEdlbkxpc3Q8KjE+OiAxMDg0NDc2NzYwLk02MzMy\n"
"NTRQODkzUjEwODRRMS5tYWkgaXMgaW4gdGhlZXZvIHN0b3JlIGFuZCB0aGUgc2VydmVyIHN0b3Jl\n"
"SnVsIDE0IDE3OjI0OjQ4IEVEVDogR2VuTGlzdDwqMT46IDIwMDQwNjIzVDA0MDEyNVotMzI1Nzct\n"
"MTAwLTEtMUBsb3VmYSBpcyBpbnRoZSBldm8gc3RvcmUgYW5kIHRoZSBzZXJ2ZXIgc3RvcmVKdWwg\n"
"MTQgMTc6MjQ6NDggRURUOiBHZW5MaXN0PCoxPjogMjAwNDA2MjNUMDQ1NDQxWi0zMjU3Ny0xMDAt\n"
"MS03QGxvdWZhIGlzIGludGhlIGV2byBzdG9yZSBhbmQgdGhlIHNlcnZlciBzdG9yZQ==\n";

    char output_stuff [] =
"Jul 14 17:24:48 EDT: WvExcConn<*1>: Applying response for co"
"mmand *Jul 14 17:24:48 EDT: UpdateItem<*1>: Reading: 61 61 6"
"1 61 61 61 61 61 61 61 6113 1 Jul 14 17:24:48 EDT: UpdateIte"
"m<*1>: Read 685 bytes of base64Jul 14 17:24:48 EDT: UpdateIt"
"em<*1>: Wrote 672 bytes into decoderJul 14 17:24:48 EDT: Upd"
"ateItem<*1>: (wrote may be less than read, due tonewline tri"
"mming)Jul 14 17:24:48 EDT: CalendarAdaptor<*1>: Told to upda"
"te item:1085093695.M734526P15428R8806Q7.maiJul 14 17:24:48 E"
"DT: ExchangeItAdaptor<*1>: Attempting to convert fromISO8859"
"-1 to UTF-8Jul 14 17:24:48 EDT: CalendarAdaptor<*1>: Got an "
"update for1085093695.M734526P15428R8806Q7.maiJul 14 17:24:48"
" EDT: GenUpdate<*1>: Finished generic UPDATE, returning 1Jul"
" 14 17:24:48 EDT: WvExcConn<*1>: Found 0 newlines and ate th"
"emJul 14 17:24:48 EDT: WvExcConn<*1>: << (* UPDATE pcolijn.C"
"alendarIPF.Appointment 1085093695.M184469P15428R5423Q1.mai)J"
"ul 14 17:24:48 EDT: WvExcConn<*1>: Applying response for com"
"mand *Jul 14 17:24:48 EDT: UpdateItem<*1>: Reading: 61 61 61"
" 61 61 61 61 61 61 61 6113 1 Jul 14 17:24:48 EDT: UpdateItem"
"<*1>: Read 685 bytes of base64Jul 14 17:24:48 EDT: UpdateIte"
"m<*1>: Wrote 672 bytes into decoderJul 14 17:24:48 EDT: Upda"
"teItem<*1>: (wrote may be less than read, due tonewline trim"
"ming)Jul 14 17:24:48 EDT: CalendarAdaptor<*1>: Told to updat"
"e item:1085093695.M184469P15428R5423Q1.maiJul 14 17:24:48 ED"
"T: ExchangeItAdaptor<*1>: Attempting to convert fromISO8859-"
"1 to UTF-8Jul 14 17:24:48 EDT: CalendarAdaptor<*1>: Got an u"
"pdate for1085093695.M184469P15428R5423Q1.maiJul 14 17:24:48 "
"EDT: GenUpdate<*1>: Finished generic UPDATE, returning 1Jul "
"14 17:24:48 EDT: WvExcConn<*1>: Found 0 newlines and ate the"
"mJul 14 17:24:48 EDT: WvExcConn<*1>: << (* UPDATE pcolijn.Ca"
"lendarIPF.Appointment 20040625T025030Z-27348-100-1-2@loufa)J"
"ul 14 17:24:48 EDT: WvExcConn<*1>: Applying response for com"
"mand *Jul 14 17:24:48 EDT: UpdateItem<*1>: Reading: 61 61 61"
" 61 61 61 61 61 61 61 6161 61 61 61 33 1 Jul 14 17:24:48 EDT"
": UpdateItem<*1>: Read 949 bytes of base64Jul 14 17:24:48 ED"
"T: UpdateItem<*1>: Wrote 932 bytes into decoderJul 14 17:24:"
"48 EDT: UpdateItem<*1>: (wrote may be less than read, due to"
"newline trimming)Jul 14 17:24:48 EDT: CalendarAdaptor<*1>: T"
"old to update item:20040625T025030Z-27348-100-1-2@loufaJul 1"
"4 17:24:48 EDT: ExchangeItAdaptor<*1>: Attempting to convert"
" fromISO8859-1 to UTF-8Jul 14 17:24:48 EDT: CalendarAdaptor<"
"*1>: Got an update for20040625T025030Z-27348-100-1-2@loufaJu"
"l 14 17:24:48 EDT: GenUpdate<*1>: Finished generic UPDATE, r"
"eturning 1Jul 14 17:24:48 EDT: WvExcConn<*1>: Found 0 newlin"
"es and ate themJul 14 17:24:48 EDT: WvExcConn<*1>: << (* UPD"
"ATE pcolijn.CalendarIPF.Appointment 1088030663.M726203P2288R"
"6280Q1.mai)Jul 14 17:24:48 EDT: WvExcConn<*1>: Applying resp"
"onse for command *Jul 14 17:24:48 EDT: UpdateItem<*1>: Readi"
"ng: 61 61 61 61 61 61 61 61 61 61 6161 61 61 61 29 1 Jul 14 "
"17:24:48 EDT: UpdateItem<*1>: Read 945 bytes of base64Jul 14"
" 17:24:48 EDT: UpdateItem<*1>: Wrote 928 bytes into decoderJ"
"ul 14 17:24:48 EDT: UpdateItem<*1>: (wrote may be less than "
"read, due tonewline trimming)Jul 14 17:24:48 EDT: CalendarAd"
"aptor<*1>: Told to update item:1088030663.M726203P2288R6280Q"
"1.maiJul 14 17:24:48 EDT: GenUpdate<*1>: Finished generic UP"
"DATE, returning 1Jul 14 17:24:48 EDT: WvExcConn<*1>: Found 0"
" newlines and ate themJul 14 17:24:48 EDT: WvExcConn<*1>: <<"
" (* UPDATE pcolijn.CalendarIPF.Appointment 20040623T045823Z-"
"32577-100-1-9@loufa)Jul 14 17:24:48 EDT: WvExcConn<*1>: Appl"
"ying response for command *Jul 14 17:24:48 EDT: UpdateItem<*"
"1>: Reading: 61 61 61 61 61 61 61 61 61 61 6161 61 61 61 61 "
"61 61 61 61 61 61 61 61 61 61 61 61 61 61 37 1 Jul 14 17:24:"
"48 EDT: UpdateItem<*1>: Read 1868 bytes of base64Jul 14 17:2"
"4:48 EDT: UpdateItem<*1>: Wrote 1836 bytes into decoderJul 1"
"4 17:24:48 EDT: UpdateItem<*1>: (wrote may be less than read"
", due tonewline trimming)Jul 14 17:24:48 EDT: CalendarAdapto"
"r<*1>: Told to update item:20040623T045823Z-32577-100-1-9@lo"
"ufaJul 14 17:24:48 EDT: ExchangeItAdaptor<*1>: Attempting to"
" convert fromISO8859-1 to UTF-8Jul 14 17:24:48 EDT: Calendar"
"Adaptor<*1>: Got an update for20040623T045823Z-32577-100-1-9"
"@loufaJul 14 17:24:48 EDT: GenUpdate<*1>: Finished generic U"
"PDATE, returning 1Jul 14 17:24:48 EDT: WvExcConn<*1>: Found "
"0 newlines and ate themJul 14 17:24:48 EDT: WvExcConn<*1>: <"
"< (* UPDATE pcolijn.CalendarIPF.Appointment 1085093695.M6966"
"42P15428R2169Q6.mai)Jul 14 17:24:48 EDT: WvExcConn<*1>: Appl"
"ying response for command *Jul 14 17:24:48 EDT: UpdateItem<*"
"1>: Reading: 61 61 61 61 61 61 61 61 61 61 6113 1 Jul 14 17:"
"24:48 EDT: UpdateItem<*1>: Read 685 bytes of base64Jul 14 17"
":24:48 EDT: UpdateItem<*1>: Wrote 672 bytes into decoderJul "
"14 17:24:48 EDT: UpdateItem<*1>: (wrote may be less than rea"
"d, due tonewline trimming)Jul 14 17:24:48 EDT: CalendarAdapt"
"or<*1>: Told to update item:1085093695.M696642P15428R2169Q6."
"maiJul 14 17:24:48 EDT: ExchangeItAdaptor<*1>: Attempting to"
" convert fromISO8859-1 to UTF-8Jul 14 17:24:48 EDT: Calendar"
"Adaptor<*1>: Got an update for1085093695.M696642P15428R2169Q"
"6.maiJul 14 17:24:48 EDT: GenUpdate<*1>: Finished generic UP"
"DATE, returning 1Jul 14 17:24:48 EDT: WvExcConn<*1>: Found 0"
" newlines and ate themJul 14 17:24:48 EDT: WvExcConn<*1>: <<"
" (* UPDATE pcolijn.CalendarIPF.Appointment 1074883675.M22449"
"7P3589R3074Q2.mai)Jul 14 17:24:48 EDT: WvExcConn<*1>: Applyi"
"ng response for command *Jul 14 17:24:48 EDT: UpdateItem<*1>"
": Reading: 61 61 61 61 61 61 61 61 61 61 6113 1 Jul 14 17:24"
":48 EDT: UpdateItem<*1>: Read 685 bytes of base64Jul 14 17:2"
"4:48 EDT: UpdateItem<*1>: Wrote 672 bytes into decoderJul 14"
" 17:24:48 EDT: UpdateItem<*1>: (wrote may be less than read,"
" due tonewline trimming)Jul 14 17:24:48 EDT: CalendarAdaptor"
"<*1>: Told to update item:1074883675.M224497P3589R3074Q2.mai"
"Jul 14 17:24:48 EDT: ExchangeItAdaptor<*1>: Attempting to co"
"nvert fromISO8859-1 to UTF-8Jul 14 17:24:48 EDT: CalendarAda"
"ptor<*1>: Got an update for1074883675.M224497P3589R3074Q2.ma"
"iJul 14 17:24:48 EDT: GenUpdate<*1>: Finished generic UPDATE"
", returning 1Jul 14 17:24:48 EDT: WvExcConn<*1>: Found 0 new"
"lines and ate themJul 14 17:24:48 EDT: WvExcConn<*1>: << (* "
"UPDATE pcolijn.CalendarIPF.Appointment 1085093696.M341377P15"
"428R6636Q12.mai)Jul 14 17:24:48 EDT: WvExcConn<*1>: Applying"
" response for command *Jul 14 17:24:48 EDT: UpdateItem<*1>: "
"Reading: 61 61 61 61 61 61 61 61 61 61 6161 17 1 Jul 14 17:2"
"4:48 EDT: UpdateItem<*1>: Read 750 bytes of base64Jul 14 17:"
"24:48 EDT: UpdateItem<*1>: Wrote 736 bytes into decoderJul 1"
"4 17:24:48 EDT: UpdateItem<*1>: (wrote may be less than read"
", due tonewline trimming)Jul 14 17:24:48 EDT: CalendarAdapto"
"r<*1>: Told to update item:1085093696.M341377P15428R6636Q12."
"maiJul 14 17:24:48 EDT: ExchangeItAdaptor<*1>: Attempting to"
" convert fromISO8859-1 to UTF-8Jul 14 17:24:48 EDT: Calendar"
"Adaptor<*1>: Got an update for1085093696.M341377P15428R6636Q"
"12.maiJul 14 17:24:48 EDT: GenUpdate<*1>: Finished generic U"
"PDATE, returning 1Jul 14 17:24:48 EDT: WvExcConn<*1>: Found "
"0 newlines and ate themJul 14 17:24:48 EDT: WvExcConn<*1>: <"
"< (* LIST pcolijn.CalendarIPF.Appointment 3)Jul 14 17:24:48 "
"EDT: WvExcConn<*1>: Applying response for command *Jul 14 17"
":24:48 EDT: GenList<*1>: 30 client itemsJul 14 17:24:48 EDT:"
" GenList<*1>: 1085093696.M341377P15428R6636Q12.mai is inthe "
"evo store and the server storeJul 14 17:24:48 EDT: GenList<*"
"1>: 1074883675.M224497P3589R3074Q2.mai is in theevo store an"
"d the server storeJul 14 17:24:48 EDT: GenList<*1>: 10850936"
"95.M696642P15428R2169Q6.mai is inthe evo store and the serve"
"r storeJul 14 17:24:48 EDT: GenList<*1>: 20040623T045823Z-32"
"577-100-1-9@loufa is inthe evo store and the server storeJul"
" 14 17:24:48 EDT: GenList<*1>: 20040625T025030Z-27348-100-1-"
"2@loufa is inthe evo store and the server storeJul 14 17:24:"
"48 EDT: GenList<*1>: 1085093695.M184469P15428R5423Q1.mai is "
"inthe evo store and the server storeJul 14 17:24:48 EDT: Gen"
"List<*1>: 1085093695.M734526P15428R8806Q7.mai is inthe evo s"
"tore and the server storeJul 14 17:24:48 EDT: GenList<*1>: 2"
"0040623T040114Z-32577-100-1-0@loufa is inthe evo store and t"
"he server storeJul 14 17:24:48 EDT: GenList<*1>: 1087340740."
"M686227P9649R8702Q0.torment is inthe evo store and the serve"
"r storeJul 14 17:24:48 EDT: GenList<*1>: 20040625T045743Z-29"
"375-100-1-0@loufa is inthe evo store and the server storeJul"
" 14 17:24:48 EDT: GenList<*1>: 1085093695.M787674P15428R3831"
"Q8.mai is inthe evo store and the server storeJul 14 17:24:4"
"8 EDT: GenList<*1>: 1084476760.M633254P893R1084Q1.mai is in "
"theevo store and the server storeJul 14 17:24:48 EDT: GenLis"
"t<*1>: 20040623T040125Z-32577-100-1-1@loufa is inthe evo sto"
"re and the server storeJul 14 17:24:48 EDT: GenList<*1>: 200"
"40623T045441Z-32577-100-1-7@loufa is inthe evo store and the"
" server store";
    
    WvBufStream input_stream;
    input_stream.write(input_stuff);

    WvBufStream *buf_stream = new WvBufStream();
    WvEncoderStream b64_stream(buf_stream);
    b64_stream.writechain.append(new WvBase64Decoder, true);
    b64_stream.auto_flush(false);

    time_t timeout = 1000;
    while (true)
    {
	WvString s(input_stream.blocking_getline(timeout));
	timeout = 10; // only allow a _long_ delay once
	if (!s) break;
	b64_stream.write(s);
    }

    WvDynBuf outbuf;
    b64_stream.read(outbuf, 8714);
    WvString output = outbuf.getstr();
//    fprintf(stderr,"We got out: %s\n", output.cstr());
    WVPASS(!strcmp(output, output_stuff));
}


WVTEST_MAIN("encoderstream eof1")
{
    WvEncoderStream s(new WvLoopback);
    s.nowrite(); // done sending
    s.blocking_getline(1000);
    WVFAIL(s.isok()); // should be eof now
}


WVTEST_MAIN("encoderstream eof2")
{
    WvEncoderStream s(new WvLoopback);
    s.write("Hello\n");
    s.write("nonewline");
    s.nowrite();
    WVPASS(s.isok());
    WVPASSEQ(s.blocking_getline(1000), "Hello");
    WVPASS(s.isok());
    WVPASSEQ(s.blocking_getline(1000), "nonewline");
    WVFAIL(s.isok());
}


class ECountStream : public WvStreamClone
{
public:
    ECountStream(IWvStream *s) : WvStreamClone(s)
        { }
    
    virtual size_t uread(void *buf, size_t len)
    {
	size_t count = WvStreamClone::uread(buf, len);
	fprintf(stderr, "uread(%d) == %d\n", (int)len, (int)count);
	return count;
    }
    
    virtual size_t uwrite(const void *buf, size_t len)
    {
	size_t count = WvStreamClone::uwrite(buf, len);
	fprintf(stderr, "uwrite(%d) == %d\n", (int)len, (int)count);
	return count;
    }
};


class LetterPlusEncoder : public WvEncoder
{
    int incr;
    
public:
    LetterPlusEncoder(int _incr) { incr = _incr; }

protected:
    virtual bool _encode(WvBuf &in, WvBuf &out, bool flush)
    {
	if (!flush) return true; // don't flush unless we have to
	size_t count = in.used();
	unsigned char *c = const_cast<unsigned char *>(in.get(count));
	for (size_t i = 0; i < count; i++)
	    if (isalpha(c[i]))
		c[i] += incr;
	out.put(c, count);
	return true;
    }
};

#if 0
# define new_enc() (new LetterPlusEncoder(1))
# define new_dec() (new LetterPlusEncoder(-1))
#else
# define new_enc() (new WvGzipEncoder(WvGzipEncoder::Deflate))
# define new_dec() (new WvGzipEncoder(WvGzipEncoder::Inflate))
#endif

// not expected to work if READAHEAD is greater than 1,
// because WvEncoderStream::inbuf can't be re-encoded if it's nonempty.
// Eventually, we'll take the extra buffering out of plain WvStream; that
// should help a bit.  In the meantime, always use a readahead of 1 if you
// expect to add encoders later.
#define READAHEAD (1)



static void closecb(int *i)
{
    (*i)++;
}


WVTEST_MAIN("encoderstream eof3")
{
    int closed = 0;
    
    {
	WvLoopback *l = new WvLoopback;
	WvEncoderStream s(l);
	s.writechain.append(new_enc(), true);
	s.readchain.append(new_dec(), true);
	s.setclosecallback(wv::bind(&closecb, &closed));
	
	s.write("Hello\n");
	s.write("nonewline");
	WVPASS(s.isok());
	WVPASSEQ(s.blocking_getline(1000), "Hello");
	WVPASS(s.isok());
	l->nowrite();
	WVPASSEQ(s.blocking_getline(1000), "nonewline");
	l->close();
	s.runonce(100);
	WVFAIL(s.isok());
	WVPASSEQ(closed, 1);
    }
    WVPASSEQ(closed, 1);
}


WVTEST_MAIN("encoderstream eof4")
{
    WvStream *x = new WvStream;
    WvEncoderStream s(x);
    WVPASS(s.isok());
    x->close();
    WVFAIL(s.isok());
}


WVTEST_MAIN("add filters midstream")
{
    WvEncoderStream s(new ECountStream(new WvLoopback));
    
    // if we don't do this, the flush() commands below should be unnecessary.
    s.delay_output(true);
    
    // this allows big kernel read() calls even though we only read one
    // byte at a time from the *decoded* input stream.  Enabling this
    // tests continue_encode() in WvEncoderChain; otherwise, continue_encode()
    // wouldn't matter.
    s.min_readsize = 1024; 
    
    s.print("First line\n");
    WVPASS("1");
    s.print("Second line\n");
    s.flush(0);
    WVPASS("2");
    s.writechain.prepend(new_enc(), true);
    s.print("Third line\n");
    WVPASS("3a");
    s.print("Third line2\n");
    s.flush(0);
    WVPASS("3b");
    s.writechain.prepend(new_enc(), true);
    s.print("Fourth line\n");
    WVPASS("4a");
    s.print("Fourth line2\n");
    s.flush(0);
    WVPASS("4b");
    WVPASSEQ(s.geterr(), 0);
    
    WVPASSEQ(s.blocking_getline(1000, '\n', READAHEAD), "First line");
    WVPASSEQ(s.blocking_getline(1000, '\n', READAHEAD), "Second line");
    WVPASS("r2");
    WVPASSEQ(s.geterr(), 0);
    s.readchain.append(new_dec(), true);
    WVPASS("e3");
    WVPASSEQ(s.blocking_getline(1000, '\n', READAHEAD), "Third line");
    WVPASSEQ(s.blocking_getline(1000, '\n', READAHEAD), "Third line2");
    WVPASSEQ(s.geterr(), 0);
    WVPASS("r3");
    s.readchain.append(new_dec(), true);
    WVPASS("e4");
    WVPASSEQ(s.blocking_getline(1000, '\n', READAHEAD), "Fourth line");
    WVPASSEQ(s.geterr(), 0);
    WVPASS("r4a");
    s.runonce(1000);
    WVPASSEQ(s.blocking_getline(1000, '\n', READAHEAD), "Fourth line2");
    WVPASSEQ(s.geterr(), 0);
    WVPASS("r4b");
    
    s.nowrite();
    s.noread();
    
    WvDynBuf remainder;
    s.read(remainder, 1024);
    WVPASSEQ(remainder.used(), 0);
    wvcon->print("Remainder: '%s'\n", remainder.getstr());
    
    WVPASSEQ(s.geterr(), 0);
    wvcon->print("Error code: '%s'\n", s.errstr());
}
#endif
