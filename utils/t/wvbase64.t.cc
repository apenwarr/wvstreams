#include <cstring>

#include "wvtest.h"
#include "wvbuf.h"
#include "wvbase64.h"
#include "wvstream.h"

#define THREE_LETTERS 		"ken"
#define THREE_LETTERS_ENC	"a2Vu"
#define FOUR_LETTERS 		"kenk"
#define FOUR_LETTERS_ENC	"a2Vuaw=="
#define FIVE_LETTERS 		"kenke"
#define FIVE_LETTERS_ENC	"a2Vua2U="
#define SIX_LETTERS 		"kenken"
#define SIX_LETTERS_ENC		"a2Vua2Vu"
#define BASE64_ALPHABET_DEC	{0, 16, 131, 16, 81, 135, 32, 146, 139, 48, 211, 143, 65, 20, 147, 81, 85, 151, 97, 150, 155, 113, 215, 159, 130, 24, 163, 146, 89, 167, 162, 154, 171, 178, 219, 175, 195, 28, 179, 211, 93, 183, 227, 158, 187, 243, 223, 191};
#define BASE64_ALPHABET		"ABCDEFGHIJKLMNOPQRSTUVWXYZ" \
				"abcdefghijklmnopqrstuvwxyz" \
				"0123456789+/"
#define BASE64_ALPHABET_ENC	"QUJDREVGR0hJSktMTU5PUFFSU1RVVldYWVphYmNkZWZnaGlqa2xtbm9wcXJzdHV2d3h5ejAxMjM0NTY3ODkrLw=="

WVTEST_MAIN("basic encoding")
{
    // always use a brand new encoder since we don't want to have to
    // deal with 'finish' issues in this WVTEST_MAIN block

    { // 3 characters... no padding on end
	WvBase64Encoder enc;
	WvString result = enc.strflushstr(THREE_LETTERS, true);
	WVPASS(result == THREE_LETTERS_ENC);
    }

    { // 4 characters... should have two pad characters 
	WvBase64Encoder enc;
	WvString result = enc.strflushstr(FOUR_LETTERS, true);
	WVPASS(result == FOUR_LETTERS_ENC);
	WVFAIL(result == "a2Vuaw="); // forgot pad
	WVFAIL(result == "a2Vuaw");  // forgot pad
	WVFAIL(result == "a2Vua");   // output if flushing didn't occur
	WVFAIL(result == "a2Vu");    // not reading enough input
    }

    { // 5 characters... should have one pad character
	WvBase64Encoder enc;
	WvString result = enc.strflushstr(FIVE_LETTERS, true);
	WVPASS(result == FIVE_LETTERS_ENC);
	WVFAIL(result == "a2Vua2U"); // forgot pad
	WVFAIL(result == "a2Vua");   // output if flushing didn't occur
	WVFAIL(result == "a2Vu");    // not reading enough input
    }

    { // 6 characters
	WvBase64Encoder enc;
	WvString result = enc.strflushstr(SIX_LETTERS, true);
	WVPASS(result == SIX_LETTERS_ENC);
    }

    { // many characters long... no newlines should be put in after 76
	// characters (see RFC2045) since this encoder is not MIME-specific
	WvBase64Encoder enc;
	WvString result = enc.strflushstr(BASE64_ALPHABET, true);
	WVPASS(result == BASE64_ALPHABET_ENC);
    }

    { // very special test that makes sure encoded output uses all possible
	// characters
	WvBase64Encoder enc;
	const char a[] = BASE64_ALPHABET_DEC;
	WvString result = enc.strflushmem(a, sizeof(a), true);
	WVPASS(result == BASE64_ALPHABET);
    }

}

WVTEST_MAIN("basic decoding")
{
    // same tests as above, but now doing the reverse operation
    { // 3 characters... no padding on end
	WvBase64Decoder dec;
	WvString result = dec.strflushstr(THREE_LETTERS_ENC, true);
	WVPASS(result == THREE_LETTERS);
    }

    { // 4 characters... should have two pad characters 
	WvBase64Decoder dec;
	WvString result = dec.strflushstr(FOUR_LETTERS_ENC, true);
	WVPASS(result == FOUR_LETTERS);
    }

    { // 5 characters... should have one pad character
	WvBase64Decoder dec;
	WvString result = dec.strflushstr(FIVE_LETTERS_ENC, true);
	WVPASS(result == FIVE_LETTERS);
    }

    { // 6 characters
	WvBase64Decoder dec;
	WvString result = dec.strflushstr(SIX_LETTERS_ENC, true);
	WVPASS(result == SIX_LETTERS);
    }


    { // many characters long...
	WvBase64Decoder dec;
	WvString result = dec.strflushstr(BASE64_ALPHABET_ENC, true);
	WVPASS(result == BASE64_ALPHABET);
    }

    { // special case...
	WvBase64Decoder dec;
	WvDynBuf dest;
	dec.flushstrbuf( BASE64_ALPHABET, dest, true );
	char a[] = BASE64_ALPHABET_DEC;
	WVPASS(dest.used() == sizeof( a )
	       && !memcmp( a, dest.get(dest.used()), sizeof(a))
	       );

    }
}

WVTEST_MAIN("encode/decode feedback loop")
{
    // Base64 is totally reversible... this test encodes a string
    // in a feedback loop n times, then decodes it n times.

    // number of times to repeatedly encode
    for (int n = 2; n < 9; n++ )
    {
	WvString orig("Milk and Cereal"), src, dest;
	src = orig;
	for (int i = 0; i < n; i++)
	{
	    WvBase64Encoder enc;
	    enc.flushstrstr(src, dest, true);
	    src = dest;
	    dest = "";
	}

	for (int i = 0; i < n; i++)
	{
	    WvBase64Decoder dec;
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
	    WvEncoder *enc = (i == 1) ? (WvEncoder *) new WvBase64Encoder : new WvBase64Decoder;
	    WvString nul, result("stuff there");
	    enc->flushstrstr(nul, result, true);

	    WVPASS(result == "stuff there");
	    delete(enc);
	}

	{ // empty WvString
	    WvEncoder *enc = (i == 1) ? (WvEncoder *) new WvBase64Encoder : new WvBase64Decoder;
	    WvString empty(""), result("stuff there");
	    enc->flushstrstr(empty, result, true);

	    WVPASS(result == "stuff there");
	    delete(enc);
	}

	{ // empty WvBuf
	    WvEncoder *enc = (i == 1) ? (WvEncoder *) new WvBase64Encoder : new WvBase64Decoder;
	    WvDynBuf empty, dest;
	    empty.zap(); // ... just to be sure
	    dest.put( "stuff there", 11 );
	    enc->encode(empty, dest, true, true);

	    WVPASS(dest.used() == 11
		   && !memcmp(dest.get(dest.used()), "stuff there", 11));
	    delete(enc);
	}
    }
}

WVTEST_MAIN("decoding whitespace")
{
    { // white space at beginning
	WvBase64Decoder dec;
	WvString a_enc("   " FOUR_LETTERS_ENC), result;
	dec.flushstrstr(a_enc,result,true);
	WVPASS(result == FOUR_LETTERS);
    }

    { // white space at end
	WvBase64Decoder dec;
	WvString a_enc(FOUR_LETTERS_ENC " \n    "), result;
	dec.flushstrstr(a_enc,result,true);
	WVPASS(result == FOUR_LETTERS);
    }

    { // white space in middle
	WvBase64Decoder dec;
	WvString a_enc("a 2  V\tu \t\n  a w   = ="), result;
	dec.flushstrstr(a_enc,result,true);
	WVPASS(result == FOUR_LETTERS);
    }

    { // all kinds of crazy whitespace everywhere
	WvBase64Decoder dec;
	WvString a_enc("a 2\v\n  V u \t  a\n\n\nw   =\r ="), result;
	dec.flushstrstr(a_enc,result,true);
	WVPASS(result == FOUR_LETTERS);
    }
}

WVTEST_MAIN("decoding invalid data")
{
    { // out of range data at beginning
	WvBase64Decoder dec;
	WvDynBuf src,dest;
	unsigned char invalid[] = {199,200,201};
	src.put(invalid, 3);
	src.put(FOUR_LETTERS_ENC, sizeof(FOUR_LETTERS_ENC)-1);

	size_t original_size = src.used();
	bool ret = dec.encode(src,dest,true,false);

	WVFAIL(dec.isok()); // read bad data
	WVPASS(ret == false && dest.used() == 0);
	// should have removed the character from the buffer
	WVPASS(src.used() == original_size - 1);
    }

    {  // out of range data in middle
	WvBase64Decoder dec;
	WvDynBuf src,dest;
	src.put("a2Vu", 4);
	src.put(200); // the invalid character
	src.put("2Vu", 3);

	bool ret = dec.encode(src,dest,true,false);

	WVFAIL(dec.isok()); // read bad data
	WVPASS(ret == false);
// this line was causing valgrind errors, but it appears to be debugging.
//	wvcon->print( "%s\n", (const char*) dest.peek( 0, dest.used() ));
	// decoded as much as possible though... a2Vu -> ken
	WVPASS(dest.used() == 3
	       && !memcmp(dest.get(dest.used()), "ken", 3));
	WVPASS(src.used() == 3); // should have removed the bad character
    }

    { // not enough equal signs.. should be two, but only one
	WvBase64Decoder dec;
	WvString src("a2Vuaw="), dest;
	bool ret = dec.flushstrstr(src, dest, true);

	WVPASS(dec.isok());
	WVPASS(ret == false);
	WVPASS(dest == "kenk"); // should have been able to decode
    }

    { // not enough equal signs.. should be one, but none
	WvBase64Decoder dec;
	WvString src("a2Vua2U"), dest;
	bool ret = dec.flushstrstr(src, dest, true);

	WVPASS(dec.isok());
	WVPASS(ret == false);
	WVPASS(dest == "kenke"); // should have been able to decode
    }
}

#define SHORT_MESSAGE		"The quick brown fox jumps over the lazy dog."
#define SHORT_MESSAGE_ENC	"VGhlIHF1aWNrIGJyb3duIGZveCBqdW1wcyBvdmVyIHRoZSBsYXp5IGRvZy4="
WVTEST_MAIN("encoding data that is streamed in")
{

    // Tests that the encoder works when not all of the data is available at
    // once.

    { // Encoder only has access to one character at a time
	WvBase64Encoder enc;
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
	WvBase64Encoder enc;
	WvDynBuf src;
	WvString result;
	src.put(THREE_LETTERS, sizeof(THREE_LETTERS) - 1);
	enc.flushbufstr(src, result, false);
	src.put(THREE_LETTERS, sizeof(THREE_LETTERS) - 1);
	enc.flushbufstr(src, result, true); // end of stream now

	WVPASS(result == SIX_LETTERS_ENC);
	WVFAIL(result == THREE_LETTERS_ENC);
    }

}

WVTEST_MAIN("decoding data that is streamed in")
{
    // same tests as above, but with decoding...
    { // one char at a time
	WvBase64Decoder dec;
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

    { // four chars at a time
	WvBase64Decoder dec;
	WvDynBuf src;
	WvString result;
	src.put(THREE_LETTERS_ENC, sizeof(THREE_LETTERS_ENC) - 1);
	dec.flushbufstr(src, result, false);
	src.put(THREE_LETTERS_ENC, sizeof(THREE_LETTERS_ENC) - 1);
	dec.flushbufstr(src, result, true); // end of stream now

	WVPASS(result == SIX_LETTERS);
	WVFAIL(result == THREE_LETTERS);
    }
}

WVTEST_MAIN("flushing")
{
    // flush is true
    {
	// Putting in 4 characters will leave 1 byte of leftover data
	// (base64 works in chunks of 3 bytes).  So result will first be
	// false.  Adding 2 more characters will give 6 characters total
	// in the input buffer (4+2).  So there's no leftover data and
	// result will be true the second time.
	
	WvBase64Encoder enc;
	WvDynBuf src, dest;
	bool result;

	src.put( "kenk", 4 );
	result = enc.encode(src, dest, true, false); // flush it
	WVFAIL(result);  // should be one byte left over...
	WVPASS(dest.used() == 5
		&& !memcmp( dest.peek( 0, dest.used() ), "a2Vua", 5));

	src.put( "en", 2 );
	result = enc.encode(src, dest, true, false); // flush it
	WVPASS(result);  // now had 4+2 = 6 bytes... no leftover data
	WVPASS(dest.used() == 8
		&& !memcmp( dest.peek( 0, dest.used() ), "a2Vua2Vu", 8));
    }

    // same test, but with flush == false
    // flush is true
    {
	WvBase64Encoder enc;
	WvDynBuf src, dest;
	bool result;

	src.put( "kenk", 4 );
	result = enc.encode(src, dest, false, false); // flush it
	WVPASS(result);  // should be one byte left over...
	WVPASS(dest.used() == 5
		&& !memcmp( dest.peek( 0, dest.used() ), "a2Vua", 5));

	src.put( "en", 2 );
	result = enc.encode(src, dest, false, false); // flush it
	WVPASS(result);  // now had 4+2 = 6 bytes... no leftover data
	WVPASS(dest.used() == 8
		&& !memcmp( dest.peek( 0, dest.used() ), "a2Vua2Vu", 8));
    }

    // same test as first, but calling flush() instead
    // flush is true
    {
	WvBase64Encoder enc;
	WvDynBuf src, dest;
	bool result;

	src.put( "kenk", 4 );
	result = enc.flush(src, dest, false); // flush it
	WVFAIL(result);  // should be one byte left over...
	WVPASS(dest.used() == 5
		&& !memcmp( dest.peek( 0, dest.used() ), "a2Vua", 5));

	src.put( "en", 2 );
	result = enc.flush(src, dest, false); // flush it
	WVPASS(result);  // now had 4+2 = 6 bytes... no leftover data
	WVPASS(dest.used() == 8
		&& !memcmp( dest.peek( 0, dest.used() ), "a2Vua2Vu", 8));
    }

}

WVTEST_MAIN("finishing")
{
    // Tests that calling finish() works correctly
    {
	WvBase64Encoder enc;
	WvString a(FOUR_LETTERS), a_enc(FOUR_LETTERS_ENC), result;
	enc.flushstrstr(a, result, true);
	WVPASS(result == a_enc);
	WVPASS(enc.isfinished());
	WVPASS(enc.isfinished() == enc.isok()); // should be able to call twice

	WvString should_be_empty("");
	bool b = enc.flushstrstr( a, should_be_empty, true ); // can't encode again
	WVPASS(b == false);
	WVPASS(should_be_empty == "");
	WVFAIL(should_be_empty == a_enc);
    }

    // Same test as above, but explicitly call finish()
    {
	WvBase64Encoder enc;
	WvDynBuf src, dest;
	src.put(FOUR_LETTERS, sizeof(FOUR_LETTERS) - 1);

	enc.encode(src, dest, true, false); // don't finish yet.
	enc.finish(dest);  // ok, NOW finish.

	WVPASS(dest.used() == 8
		&& !memcmp( dest.peek( 0, dest.used() ), FOUR_LETTERS_ENC, 8));
	WVPASS(enc.isfinished());
	WVPASS(enc.isfinished() == enc.isok()); // should be able to call twice

	// shouldn't be able to encode again
	dest.zap();
	bool b = enc.encode( src, dest, true, true );
	WVPASS(b == false);
	WVPASS(dest.used() == 0);
    }

    // Now we do the same two tests with Decoders
    {
	WvBase64Decoder dec;
	WvString a(FOUR_LETTERS_ENC), a_dec(FOUR_LETTERS), result;
	dec.flushstrstr(a, result, true);
	WVPASS(result == a_dec);
	WVPASS(dec.isfinished());
	WVPASS(dec.isfinished() == dec.isok()); // should be able to call twice

	WvString should_be_empty("");
	bool b = dec.flushstrstr( a, should_be_empty, true ); // can't encode again
	WVPASS(b == false);
	WVPASS(should_be_empty == "");
	WVFAIL(should_be_empty == a_dec);
    }

    {
	WvBase64Decoder dec;
	WvDynBuf src, dest;
	src.put(FOUR_LETTERS_ENC, sizeof(FOUR_LETTERS_ENC) - 1);

	dec.encode(src, dest, true, false); // don't finish yet.
	dec.finish(dest);  // ok, NOW finish.

	WVPASS(dest.used() == 4
		&& !memcmp( dest.peek( 0, dest.used() ), FOUR_LETTERS, 4));
	WVPASS(dec.isfinished());
	WVPASS(dec.isfinished() == dec.isok()); // should be able to call twice

	// shouldn't be able to encode again
	dest.zap();
	bool b = dec.encode( src, dest, true, true );
	WVPASS(b == false);
	WVPASS(dest.used() == 0);
    }
}

WVTEST_MAIN("resetting")
{
    { // call finish, make sure it was finished, then reset it and
	// use it again
	WvBase64Encoder enc;
	WvDynBuf temp;
	WvString b(SIX_LETTERS), b_enc(SIX_LETTERS_ENC);
	WvString should_be_empty("");

	enc.finish(temp);
	WVPASS(enc.isfinished());

	// ensure finished

	bool ret = enc.flushstrstr( b, should_be_empty, true ); // can't encode again
	WVPASS(ret == false);
	WVPASS(should_be_empty == "");
	WVFAIL(should_be_empty == b_enc);

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
	WvBase64Decoder dec;
	WvDynBuf temp;
	WvString b(SIX_LETTERS), b_enc(SIX_LETTERS_ENC);
	WvString should_be_empty("");

	dec.finish(temp);
	WVPASS(dec.isfinished());

	// ensure finished

	bool ret = dec.flushstrstr( b_enc, should_be_empty, true ); // can't encode again
	WVPASS(ret == false);
	WVPASS(should_be_empty == "");
	WVFAIL(should_be_empty == b);

	dec.reset();
	WVFAIL(dec.isfinished());

	WvString result("");
	dec.flushstrstr(b_enc, result, true);
	wvcon->print( "%s", result );
	WVPASS(result == b);
	WVPASS(dec.isfinished());
    }

}
