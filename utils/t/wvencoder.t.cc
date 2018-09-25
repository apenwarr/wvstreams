#include "wvencoder.h"
#include "wvtest.h"

// BEGIN encodertest.cc definition

WVTEST_MAIN("encodertest.cc")
{   WvEncoderChain encoderChain;

    WvNullEncoder stackNull;
    WvPassthroughEncoder stackPass;
    WvNullEncoder *heapNull = new WvNullEncoder();
    WvPassthroughEncoder *heapPass = new WvPassthroughEncoder();

    encoderChain.append(&stackNull, false);
    encoderChain.prepend(&stackPass, false);
    encoderChain.append(heapNull, true);
    encoderChain.prepend(heapPass, false);

    {
	// Simple tests on empty encoders
	WVPASS(stackNull.isok());
	WVPASS(heapPass->isok());
	WVFAIL(stackPass.isfinished());
	WVPASS(heapNull->geterror() == NULL);
	WVPASS(encoderChain.isok());
	WVFAIL(encoderChain.isfinished());
	encoderChain.unlink(heapPass);
    }
    
    {
	// Simple tests with finishing an encoder and simple encoding
	WvDynBuf inbuf;
	inbuf.putstr("Luke is really cool");
	WvDynBuf outbuf;
	WVPASS(heapPass->encode(inbuf, outbuf));
	WVPASS(outbuf.getstr() == "Luke is really cool");
	printf("%d\n", (int)heapPass->bytes_processed());
	WVPASS(heapPass->bytes_processed() == 19);
	WVFAIL(heapPass->isfinished());
	inbuf.putstr("Buffers work kinda weird");
	WVPASS(stackNull.encode(inbuf, outbuf, false, true));
	WVPASS(inbuf.getstr() == WvString(""));
	WVPASS(stackNull.isfinished());
	WVFAIL(stackNull.encode(inbuf, outbuf));
    }

    {
	/* Testing some of the built-in encoding functions for a WvEncoder
	* This section is for everything but buffer-buffer encoding.
	* Note it mostly tests functions found in WvEncoder itself,
	* a WvPassthroughEncoder is used only to verify results. */
	WvPassthroughEncoder poorSap;
	size_t paramSize = 12;
	char testingMem[12] = {0};
	char testingMem2[12] = {0};
	WVPASS(poorSap.flushstrmem(WvString("Testing"), 
	    testingMem, &paramSize));
	WVPASS(!strcmp(testingMem, "Testing"));
	WVPASS(paramSize == 7);
	WvString reverse(poorSap.strflushmem(testingMem, paramSize));
	WVPASS(reverse == "Testing");
	WvDynBuf outbuf;
	WVPASS(poorSap.flushstrbuf(reverse, outbuf));
	WVPASS(outbuf.getstr() == "Testing");
	WvString anotherString(poorSap.strflushstr(reverse));
	WVPASS(anotherString == "Testing");
	outbuf.zap();
	WVPASS(poorSap.flushmembuf(testingMem, 12, outbuf));
	WVPASS(outbuf.getstr() == "Testing");
	WVPASS(poorSap.flushmemmem(testingMem, 
	    paramSize, testingMem2, &paramSize, true));
	WVPASS(!strcmp(testingMem, "Testing"));
	WVPASS(!strcmp(testingMem2, "Testing"));
	WVFAIL(poorSap.flushmembuf(testingMem, 12, outbuf));
	WVPASS(poorSap.reset());
	WVPASS(poorSap.isok() && !poorSap.isfinished());
    }

    {
	//individual encoder encoding tests
	WvNullEncoder nullie;
	WvPassthroughEncoder passie;
	WvDynBuf input;
	WvDynBuf output;
	input.putstr("Test");
	WVPASS(passie.encode(input, output));
	WVPASS(input.getstr() == "");
	input.putstr("Another");
	WVPASS(passie.encode(input, output));
	WVPASS(passie.bytes_processed() == 11);
	input.putstr("This will die");
	WVPASS(nullie.encode(input, output, true, true));
	WVPASS(output.getstr() == "TestAnother");
	WVPASS(input.getstr() == "");
	WVPASS(nullie.reset());
	WVPASS(nullie.encode(input, output, true, false));
	WVFAIL(nullie.isfinished());
    }

    {
	//encoder chain tests.
	WvDynBuf input;
	WvDynBuf output;
	input.putstr("Final test");
	heapPass->reset();
	WVPASS(stackPass.encode(input, output));
	WVFAIL(stackNull.encode(input, output));
	WVFAIL(encoderChain.encode(input, output));
	WVPASS(stackPass.bytes_processed() == 10);
	WVPASS(heapPass->bytes_processed() == 0);
	input.putstr("Whatchamahuh?");
	WVPASS(stackPass.encode(input, output, true, false));
	WVPASS(heapNull->encode(input, output, true, false));
	encoderChain.unlink(&stackNull);
	WVFAIL(encoderChain.isfinished());
	WVPASS(encoderChain.encode(input, output, true, false));
	WVPASS(encoderChain.isok());
	WVPASS(encoderChain.geterror() == NULL);
	WvPassthroughEncoder* temp = new WvPassthroughEncoder();
	encoderChain.append(temp, false);
	WVPASS(encoderChain.flush(input, output, true));
	WVPASS(stackPass.isfinished());
	WVPASS(heapNull->isfinished());
	encoderChain.zap();
	WVPASS(temp->isfinished());
	WVPASS(encoderChain.isfinished());
	input.putstr("Auf Wiedershen");
	WVPASS(encoderChain.reset());
	output.zap();
	WVPASS(encoderChain.encode(input, output));
	WVPASS(output.getstr() == "Auf Wiedershen");
	delete temp;
    }
   
    // Valgrind will nicely pick up to make sure everything is freed here.
    // This element has to be deleted manually because... well; I want that.
    delete heapPass;
}

// END encodertest.cc definition
