#include <stdio.h>

#ifdef _WIN32
#define snprintf _snprintf
#endif

#include "wvautoconf.h"
#ifndef HAVE_ALLOCA
# ifdef __GNUC__
#  define alloca __builtin_alloca
# else
#  ifdef _MSC_VER
#   include <malloc.h>
#   define alloca _alloca
#  else
#   if HAVE_ALLOCA_H
#    include <alloca.h>
#   else
#    ifdef _AIX
#pragma alloca
#    else
#     ifndef alloca /* predefined by HP cc +Olibcalls */
char *alloca ();
#     endif
#    endif
#   endif
#  endif
# endif
#endif

#include "wvtest.h"
#include "wvbuf.h"
#include "wvhex.h"
#include "wvstream.h"

#define THREE_LETTERS 		"abz"
#define THREE_LETTERS_ENC_LC	"61627a"
#define THREE_LETTERS_ENC_UC	"61627A"

#define FOUR_LETTERS 		"a+cz"
#define FOUR_LETTERS_ENC_LC	"612b637a"
#define FOUR_LETTERS_ENC_UC	"612B637A"

#define FIVE_LETTERS 		"abcdz"
#define FIVE_LETTERS_ENC_LC	"616263647a"
#define FIVE_LETTERS_ENC_UC	"616263647A"

#define SIX_LETTERS 		"ab^dez"
#define SIX_LETTERS_ENC_LC	"61625e64657a"
#define SIX_LETTERS_ENC_UC	"61625E64657A"

WVTEST_MAIN("basic encoding lowercase")
{
    // always use a brand new encoder since we don't want to have to
    // deal with 'finish' issues in this WVTEST_MAIN block

    const char* INPUTS[] = {THREE_LETTERS,
			    FOUR_LETTERS,
			    FIVE_LETTERS,
			    SIX_LETTERS};

    const char* OUTPUTS[] = {THREE_LETTERS_ENC_LC,
			     FOUR_LETTERS_ENC_LC,
			     FIVE_LETTERS_ENC_LC,
			     SIX_LETTERS_ENC_LC};

    const unsigned int num_tests = 4;

    for (unsigned int i = 0; i < num_tests; i++)
    {
	WvHexEncoder enc;
	WvString result = enc.strflushstr(INPUTS[i], true);
	WVPASS(result == OUTPUTS[i]);
    }

    { // encode all possible characters
	WvHexEncoder enc;
	WvDynBuf src,dest;
	for (unsigned int c = 0; c <= 255; c++)
	    src.put((unsigned char) c);

	bool result = enc.encode(src,dest,true,true);
	WVPASS(result == true);
	WVPASS(dest.used() == 512);

	bool encoded_correctly = true;
	unsigned int n = dest.used() / 2;
	for (unsigned int i = 0; i < n; i++)
	{
	    char buf[3];
	    buf[0] = dest.peek(i*2);
	    buf[1] = dest.peek(i*2 + 1);
	    buf[2] = '\0';

	    long int curr = strtol(buf, NULL, 16);
	    if ((unsigned int) curr != i)
	    {
		encoded_correctly = false;
		break;
	    }
	}
	WVPASS(encoded_correctly);
    }

}

WVTEST_MAIN("basic encoding uppercase")
{
    // always use a brand new encoder since we don't want to have to
    // deal with 'finish' issues in this WVTEST_MAIN block

    const char* INPUTS[] = {THREE_LETTERS,
			    FOUR_LETTERS,
			    FIVE_LETTERS,
			    SIX_LETTERS};

    const char* OUTPUTS[] = {THREE_LETTERS_ENC_UC,
			     FOUR_LETTERS_ENC_UC,
			     FIVE_LETTERS_ENC_UC,
			     SIX_LETTERS_ENC_UC};

    const unsigned int num_tests = 4;

    for (unsigned int i = 0; i < num_tests; i++)
    {
	WvHexEncoder enc(true);
	WvString result = enc.strflushstr(INPUTS[i], true);
	WVPASS(result == OUTPUTS[i]);
    }

    { // encode all possible characters
	WvHexEncoder enc(true);
	WvDynBuf src,dest;
	for (unsigned int c = 0; c <= 255; c++)
	    src.put((unsigned char) c);

	bool result = enc.encode(src,dest,true,true);
	WVPASS(result == true);
	WVPASS(dest.used() == 512);

	bool encoded_correctly = true;
	unsigned int n = dest.used() / 2;
	for (unsigned int i = 0; i < n; i++)
	{
	    char buf[3];
	    buf[0] = dest.peek(i*2);
	    buf[1] = dest.peek(i*2 + 1);
	    buf[2] = '\0';

	    long int curr = strtol(buf, NULL, 16);
	    if ((unsigned int) curr != i)
	    {
		encoded_correctly = false;
		break;
	    }
	}
	WVPASS(encoded_correctly);
    }

}

WVTEST_MAIN("legacy hexify function")
{
    // Note that if ibuf is of length n, then obuf must be of
    // length 2*n + 1 (two bytes for each byte in ibuf, plus
    // terminating null.

    // basic tests
    const char* INPUTS[] = {THREE_LETTERS,
			    FOUR_LETTERS,
			    FIVE_LETTERS,
			    SIX_LETTERS};

    const char* OUTPUTS[] = {THREE_LETTERS_ENC_LC,
			     FOUR_LETTERS_ENC_LC,
			     FIVE_LETTERS_ENC_LC,
			     SIX_LETTERS_ENC_LC};

    unsigned int num_tests = 4;
    for (unsigned int i = 0; i < num_tests; i++)
    {
	const char *ibuf = INPUTS[i];
	size_t n = strlen(ibuf);
	char *obuf = (char *) alloca(2*n + 1);
	hexify(obuf, ibuf, n);
	WVPASS(strcmp(obuf, OUTPUTS[i])==0);
    }

    // make sure that 3rd parameter is handled correctly
    {
	const char *ibuf = THREE_LETTERS;
	const size_t n = 2;
	char obuf[2*n + 1];
	memset(obuf, 'K', n); // to be sure that final \0 is put in later
	hexify(obuf, ibuf, n);
	WVPASS(strlen(obuf) == 2*n
	       && strncmp(obuf, THREE_LETTERS_ENC_LC,2*n) == 0);
    }

    {
	const char *ibuf = SIX_LETTERS;
	const size_t n = 1;
	char obuf[2*n + 1];
	memset(obuf, 'K', n); // to be sure that final \0 is put in later
	hexify(obuf, ibuf, n);
	WVPASS(strlen(obuf) == 2*n
	       && strncmp(obuf, SIX_LETTERS_ENC_LC,2*n) == 0);
    }

}

WVTEST_MAIN("basic decoding lowercase")
{
    // same tests as above, but now doing the reverse operation
    { // 3 characters...
	WvHexDecoder dec;
	WvString result = dec.strflushstr(THREE_LETTERS_ENC_LC, true);
	WVPASS(result == THREE_LETTERS);
    }

    { // 4 characters...
	WvHexDecoder dec;
	WvString result = dec.strflushstr(FOUR_LETTERS_ENC_LC, true);
	WVPASS(result == FOUR_LETTERS);
    }

    { // 5 characters...
	WvHexDecoder dec;
	WvString result = dec.strflushstr(FIVE_LETTERS_ENC_LC, true);
	WVPASS(result == FIVE_LETTERS);
    }

    { // 6 characters...
	WvHexDecoder dec;
	WvString result = dec.strflushstr(SIX_LETTERS_ENC_LC, true);
	WVPASS(result == SIX_LETTERS);
    }

    { // decode all possible characters
	WvHexDecoder dec;
	WvDynBuf src,dest;
	for (unsigned int c = 0; c <= 255; c++)
	{
	    char buf[3];
	    snprintf(buf, 3, "%02x", c);
	    src.put(buf, 2);
	}
	dec.encode(src,dest,true,true);

	WVPASS(dest.used() == 256);
	bool decoded_correctly = true;
	for (unsigned int i = 0; i < dest.used(); i++)
	{
	    if (dest.peek(i) != i)
	    {
		decoded_correctly = false;
		break;
	    }
	}
	WVPASS(decoded_correctly);
    }
}

WVTEST_MAIN("basic decoding uppercase")
{
    // same tests as above, but now doing the reverse operation
    { // 3 characters...
	WvHexDecoder dec;
	WvString result = dec.strflushstr(THREE_LETTERS_ENC_UC, true);
	WVPASS(result == THREE_LETTERS);
    }

    { // 4 characters...
	WvHexDecoder dec;
	WvString result = dec.strflushstr(FOUR_LETTERS_ENC_UC, true);
	WVPASS(result == FOUR_LETTERS);
    }

    { // 5 characters...
	WvHexDecoder dec;
	WvString result = dec.strflushstr(FIVE_LETTERS_ENC_UC, true);
	WVPASS(result == FIVE_LETTERS);
    }

    { // 6 characters...
	WvHexDecoder dec;
	WvString result = dec.strflushstr(SIX_LETTERS_ENC_UC, true);
	WVPASS(result == SIX_LETTERS);
    }

    { // decode all possible characters
	WvHexDecoder dec;
	WvDynBuf src,dest;
	for (unsigned int c = 0; c <= 255; c++)
	{
	    char buf[3];
	    snprintf(buf, 3, "%02X", c);
	    src.put(buf, 2);
	}
	dec.encode(src,dest,true,true);

	WVPASS(dest.used() == 256);
	bool decoded_correctly = true;
	for (unsigned int i = 0; i < dest.used(); i++)
	{
	    if (dest.peek(i) != i)
	    {
		decoded_correctly = false;
		break;
	    }
	}
	WVPASS(decoded_correctly);
    }
}


// void unhexify(unsigned char *obuf, char *ibuf)
//
// obuf must be a buffer large enough to contain the entire binary
// output string; you can calculate this size with (strlen(ibuf) / 2).
// obuf should not be automatically NUL-terminated.
//
// factor out testing of upper case and lowercase
void legacy_unhexify_test(const char *INPUTS[],
			  const char *OUTPUTS[],
			  const unsigned int num_tests)
{
    // test basic decoding with a string that's initially zeros
    for (unsigned int i = 0; i < num_tests; i++)
    {
	const char* ibuf = INPUTS[i];
	const size_t n = (strlen(ibuf)/2) + 1;
	char *obuf = (char *) alloca(n);
	memset(obuf, 0, n); // make sure it's all zeros
	unhexify(obuf, ibuf);
	WVPASS(strcmp(obuf, OUTPUTS[i]) == 0);
    }

    // test that obuf is not nul terminated
    for (unsigned int i = 0; i < num_tests; i++)
    {
	const char* ibuf = INPUTS[i];
	const size_t n = (strlen(ibuf)/2) + 3;
	char *obuf = (char *) alloca(n);
	memset(obuf, 'K', n); // fill it up with something non-zero
	unhexify(obuf+1, ibuf); // allow one 'K' at the beginning
	WvString actual_output("K%sKK", OUTPUTS[i]);
	WVPASS(strncmp(obuf, actual_output.cstr(),n) == 0);
    }
}

WVTEST_MAIN("legacy unhexify function (lowercase input)")
{
    const char* INPUTS[] = {THREE_LETTERS_ENC_LC,
			    FOUR_LETTERS_ENC_LC,
			    FIVE_LETTERS_ENC_LC,
			    SIX_LETTERS_ENC_LC};

    const char* OUTPUTS[] = {THREE_LETTERS,
			     FOUR_LETTERS,
			     FIVE_LETTERS,
			     SIX_LETTERS};

    legacy_unhexify_test(INPUTS, OUTPUTS, 4);

}

WVTEST_MAIN("legacy unhexify function (uppercase input)")
{
    const char* INPUTS[] = {THREE_LETTERS_ENC_UC,
			    FOUR_LETTERS_ENC_UC,
			    FIVE_LETTERS_ENC_UC,
			    SIX_LETTERS_ENC_UC};

    const char* OUTPUTS[] = {THREE_LETTERS,
			     FOUR_LETTERS,
			     FIVE_LETTERS,
			     SIX_LETTERS};

    legacy_unhexify_test(INPUTS, OUTPUTS, 4);

}

WVTEST_MAIN("encode/decode feedback loop")
{
    // Encodes a string in a feedback loop n times, then decodes it n times.

    // number of times to repeatedly encode
    for (int n = 2; n < 9; n++ )
    {
	WvString orig("Milk and Cereal"), src, dest;
	src = orig;
	for (int i = 0; i < n; i++)
	{
	    WvHexEncoder enc;
	    enc.flushstrstr(src, dest, true);
	    src = dest;
	    dest = "";

	}

	for (int i = 0; i < n; i++)
	{
	    WvHexDecoder dec;
	    dec.flushstrstr(src, dest, true);
	    src = dest;
	    dest = "";
	}

	WVPASS( src == orig );
    }
}


WVTEST_MAIN("nothing to encode/decode")
{
    for (int i = 1; i <= 2; i++ )
    {
	{ // "null" WvString
	    WvEncoder *enc = (i == 1) ? (WvEncoder *)new WvHexEncoder : new WvHexDecoder;
	    WvString nul, result("stuff there");
	    enc->flushstrstr(nul, result, true);

	    WVPASS(result == "stuff there");
	    delete(enc);
	}

	{ // empty WvString
	    WvEncoder *enc = (i == 1) ? (WvEncoder *)new WvHexEncoder : new WvHexDecoder;
	    WvString empty(""), result("stuff there");
	    enc->flushstrstr(empty, result, true);

	    WVPASS(result == "stuff there");
	    delete(enc);
	}

	{ // empty WvBuf
	    WvEncoder *enc = (i == 1) ? (WvEncoder *)new WvHexEncoder : new WvHexDecoder;
	    WvDynBuf empty, dest;
	    empty.zap(); // ... just to be sure
	    dest.put("stuff there", 11);
	    enc->encode(empty, dest, true, true);

	    WVPASS(dest.used() == 11
		   && !memcmp(dest.get(dest.used()), "stuff there", 11));
	    delete(enc);
	}

	{ // empty char*
	    WvEncoder *enc = (i == 1) ? (WvEncoder *)new WvHexEncoder : new WvHexDecoder;
	    char *empty = "";
	    WvDynBuf dest;
	    dest.put("stuff there",11);
	    enc->flushmembuf(empty, 0, dest, true);
	    WVPASS(dest.used() == 11
		   && !memcmp(dest.get(dest.used()), "stuff there", 11));
	    delete(enc);
	}
    }
}

WVTEST_MAIN("decoding whitespace")
{
    { // white space at beginning
	WvHexDecoder dec;
	WvString a_enc("   " FOUR_LETTERS_ENC_LC), result;
	dec.flushstrstr(a_enc,result,true);
	WVPASS(result == FOUR_LETTERS);
    }

    { // white space at end
	WvHexDecoder dec;
	WvString a_enc(FOUR_LETTERS_ENC_LC " \n    "), result;
	dec.flushstrstr(a_enc,result,true);
	WVPASS(result == FOUR_LETTERS);
    }

    { // white space in middle
	WvHexDecoder dec;
	WvString a_enc("6 1  2\tb \t\n  6 3   7 a"), result;
	dec.flushstrstr(a_enc,result,true);
	WVPASS(result == FOUR_LETTERS);
    }

    { // all kinds of crazy whitespace everywhere
	WvHexDecoder dec;
	WvString a_enc("6 1\v\n  2 b \t  6\n\n\n3   7\r a"), result;
	dec.flushstrstr(a_enc,result,true);
	WVPASS(result == FOUR_LETTERS);
    }
}

WVTEST_MAIN("decoding invalid data")
{
    { // out of range data at beginning
	WvHexDecoder dec;
	WvDynBuf src,dest;
	unsigned char invalid[] = {'G', 'h', 'i'};
	src.put(invalid, 3);
	src.put(FOUR_LETTERS_ENC_LC, sizeof(FOUR_LETTERS_ENC_LC)-1);

	unsigned int original_size = src.used();
	bool ret = dec.encode(src,dest,true,false);

	WVFAIL(dec.isok()); // read bad data
	WVPASS(ret == false && dest.used() == 0);
	// should have removed the character from the buffer
	WVPASS(src.used() == original_size - 1);
    }

    {  // out of range data in middle
	WvHexDecoder dec;
	WvDynBuf src,dest;
	src.put("612b", 4);
	src.put('K'); // the invalid character
	src.put("637A", 4);

	bool ret = dec.encode(src,dest,true,false);

	WVFAIL(dec.isok()); // read bad data
	WVPASS(ret == false);

	// decoded as much as possible though... a2Vu -> ken
	WVPASS(dest.used() == 2
	       && !memcmp(dest.peek(0,dest.used()), "a+", 2));
	WVPASS(src.used() == 4); // 4 chars left since should have removed
				 // the bad character
    }
}

#define SHORT_MESSAGE		"The quick brown fox jumps over the lazy dog."
#define SHORT_MESSAGE_ENC	"54686520717569636b2062726f776e20666f78206a756d7073206f76657220746865206c617a7920646f672e"
WVTEST_MAIN("encoding data that is streamed in")
{

    // Tests that the encoder works when not all of the data is available at
    // once.

    { // Encoder only has access to one character at a time
	WvHexEncoder enc;
	WvDynBuf src;
	WvString a(SHORT_MESSAGE),result;
	for (unsigned int i = 0; i < a.len(); i++)
	{
	    src.put(a.edit()[i]);
	    enc.flushbufstr(src, result, false);
	}
	enc.flushbufstr(src, result, true);
	WVPASS(result == SHORT_MESSAGE_ENC);
    }

    { // encoder has access to three letters at a time
	WvHexEncoder enc;
	WvDynBuf src;
	WvString result;
	src.put(THREE_LETTERS, sizeof(THREE_LETTERS) - 1);
	enc.flushbufstr(src, result, false);
	src.put(THREE_LETTERS, sizeof(THREE_LETTERS) - 1);
	enc.flushbufstr(src, result, true); // end of stream now

	WVPASS(result == THREE_LETTERS_ENC_LC THREE_LETTERS_ENC_LC);
    }

}

WVTEST_MAIN("decoding data that is streamed in")
{
    // same tests as above, but with decoding...
    { // one char at a time
	WvHexDecoder dec;
	WvDynBuf src;
	WvString a(SHORT_MESSAGE_ENC),result;
	for (unsigned int i = 0; i < a.len(); i++)
	{
	    src.put(a.edit()[i]);
	    dec.flushbufstr(src, result, false);
	}
	dec.flushbufstr(src, result, true);
	WVPASS(result == SHORT_MESSAGE);
    }

    { // six chars at a time
	WvHexDecoder dec;
	WvDynBuf src;
	WvString result;
	src.put(THREE_LETTERS_ENC_LC, sizeof(THREE_LETTERS_ENC_LC) - 1);
	dec.flushbufstr(src, result, false);
	src.put(THREE_LETTERS_ENC_LC, sizeof(THREE_LETTERS_ENC_LC) - 1);
	dec.flushbufstr(src, result, true); // end of stream now

	WVPASS(result == THREE_LETTERS THREE_LETTERS);
    }
}

WVTEST_MAIN("flushing")
{
    // flush is true
    {

	WvHexEncoder enc;
	WvDynBuf src, dest;
	bool result;

	src.put( "a+", 2 );
	result = enc.encode(src, dest, true, false); // flush it
	WVPASS(result);  // shouldn't have anything buffered
	WVPASS(dest.used() == 4
		&& !memcmp( dest.peek( 0, dest.used() ), "612b", 4));

	src.put( "cz", 2 );
	result = enc.encode(src, dest, true, false); // flush it
	WVPASS(result);  // still nothing buffered
	WVPASS(dest.used() == 8
		&& !memcmp( dest.peek( 0, dest.used() ), "612b637a", 8));
    }

    // same test, but with flush == false
    {
	WvHexEncoder enc;
	WvDynBuf src, dest;
	bool result;

	src.put( "a+", 2 );
	result = enc.encode(src, dest, false, false); // flush it
	WVPASS(result);  // shouldn't have anything buffered
	WVPASS(dest.used() == 4
		&& !memcmp( dest.peek( 0, dest.used() ), "612b", 4));

	src.put( "cz", 2 );
	result = enc.encode(src, dest, false, false); // flush it
	WVPASS(result);  // still nothing buffered
	WVPASS(dest.used() == 8
		&& !memcmp( dest.peek( 0, dest.used() ), "612b637a", 8));
    }

    // same test as first, but calling flush() instead
    // flush is true
    {
	WvHexEncoder enc;
	WvDynBuf src, dest;
	bool result;

	src.put("a+", 2 );
	result = enc.flush(src, dest, false); // flush it
	WVPASS(result);  // shouldn't have anything buffered
	WVPASS(dest.used() == 4
		&& !memcmp( dest.peek( 0, dest.used() ), "612b", 4));

	src.put( "cz", 2 );
	result = enc.flush(src, dest, false); // flush it
	WVPASS(result);
	WVPASS(dest.used() == 8
		&& !memcmp( dest.peek( 0, dest.used() ), "612b637a", 8));
    }

}

WVTEST_MAIN("finishing")
{
    // Tests that calling finish() works correctly
    {
	WvHexEncoder enc;
	WvString a(FOUR_LETTERS), a_enc(FOUR_LETTERS_ENC_LC), result;
	enc.flushstrstr(a, result, true);
	WVPASS(result == a_enc);
	WVPASS(enc.isfinished());
	WVPASS(enc.isfinished() == enc.isok()); // should be able to call twice

	WvString should_be_empty("");
	bool b = enc.flushstrstr( a, should_be_empty, true ); // can't encode again
	WVFAIL(b);
	WVPASS(should_be_empty == "");
	WVFAIL(should_be_empty == a_enc);
    }

    // Same test as above, but explicitly call finish()
    {
	WvHexEncoder enc;
	WvDynBuf src, dest;
	src.put(FOUR_LETTERS, sizeof(FOUR_LETTERS) - 1);

	enc.encode(src, dest, true, false); // don't finish yet.
	enc.finish(dest);  // ok, NOW finish.

	WVPASS(dest.used() == 8
		&& !memcmp( dest.peek( 0, dest.used() ), FOUR_LETTERS_ENC_LC, 8));
	WVPASS(enc.isfinished());
	WVPASS(enc.isfinished() == enc.isok()); // should be able to call twice

	// shouldn't be able to encode again
	dest.zap();
	bool b = enc.encode(src, dest, true, true);
	WVFAIL(b);
	WVPASS(dest.used() == 0);
    }

    // Now we do the same two tests with Decoders
    {
	WvHexDecoder dec;
	WvString a(FOUR_LETTERS_ENC_LC), a_dec(FOUR_LETTERS), result;
	dec.flushstrstr(a, result, true);
	WVPASS(result == a_dec);
	WVPASS(dec.isfinished());
	WVPASS(dec.isfinished() == dec.isok()); // should be able to call twice

	WvString should_be_empty("");
	bool b = dec.flushstrstr( a, should_be_empty, true ); // can't encode again
	WVFAIL(b);
	WVPASS(should_be_empty == "");
	WVFAIL(should_be_empty == a_dec);
    }

    {
	WvHexDecoder dec;
	WvDynBuf src, dest;
	src.put(FOUR_LETTERS_ENC_LC, sizeof(FOUR_LETTERS_ENC_LC) - 1);

	dec.encode(src, dest, true, false); // don't finish yet.
	dec.finish(dest);  // ok, NOW finish.

	WVPASS(dest.used() == 4
		&& !memcmp( dest.peek( 0, dest.used() ), FOUR_LETTERS, 4));
	WVPASS(dec.isfinished());
	WVPASS(dec.isfinished() == dec.isok()); // should be able to call twice

	// shouldn't be able to encode again
	dest.zap();
	bool b = dec.encode(src, dest, true, true);
	WVFAIL(b);
	WVPASS(dest.used() == 0);
    }
}

WVTEST_MAIN("resetting")
{
    { // call finish, make sure it was finished, then reset it and
	// use it again
	WvHexEncoder enc;
	WvDynBuf temp;
	WvString b(SIX_LETTERS), b_enc(SIX_LETTERS_ENC_LC);
	WvString should_be_empty("");

	enc.finish(temp);
	WVPASS(enc.isfinished());

	// ensure finished

	bool ret = enc.flushstrstr( b, should_be_empty, true ); // can't encode again
	WVFAIL(ret);
	WVPASS(should_be_empty == "");

	enc.reset();
	WVFAIL(enc.isfinished());

	WvString result("");
	enc.flushstrstr(b, result, true);
	WVPASS(result == b_enc);
	WVPASS(enc.isfinished());
    }

    // same test as above with Decoder
    { // call finish, make sure it was finished, then reset it and
	// use it again
	WvHexDecoder dec;
	WvDynBuf temp;
	WvString b(SIX_LETTERS), b_enc(SIX_LETTERS_ENC_LC);
	WvString should_be_empty("");

	dec.finish(temp);
	WVPASS(dec.isfinished());

	// ensure finished

	bool ret = dec.flushstrstr( b_enc, should_be_empty, true ); // can't encode again
	WVFAIL(ret);
	WVPASS(should_be_empty == "");

	dec.reset();
	WVFAIL(dec.isfinished());

	WvString result("");
	dec.flushstrstr(b_enc, result, true);
	WVPASS(result == b);
	WVPASS(dec.isfinished());
    }

}

