#include "wvlog.h"
#include "wvdigest.h"
#include "wvhex.h"
#include "wvfile.h"

// Quick program to test the MD5 Routines...

int main()
{
	free(malloc(1)); // For Electric Fence...

	WvString request = "md5test";
        WvDynamicBuffer md5buf;
        WvMD5Digest md5;

	WvLog log("MD5test", WvLog::Info);
	log("Starting...\n");

        WvFile file(request, O_RDONLY);
        WvDynamicBuffer filein;
        while (file.isok())
        {
            file.read(filein, 1024);
            md5.encode(filein, md5buf);
        }
        md5.finish(md5buf);
        WvString md5str = WvHexEncoder().strflush(md5buf, true);
	
        log("%s -> %s\n", request, md5str);

	log("Done...\n");
}
