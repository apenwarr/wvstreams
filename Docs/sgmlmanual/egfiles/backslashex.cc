#include "wvbackslash.h"
#include "wvbufbase.h"

int main()
{
   WvBackslashEncoder *enc;
   enc = new WvBackslashEncoder("abcd");
   // enc contains the letters you want to escape (add a backslash to)

   // If you want a decoder, then enc has to be initialiazed like this:
   // enc = new WvBackslashDecoder();

   WvInPlaceBuf to_encode(20);
   WvInPlaceBuf out(40);

   to_encode.put("Test abcdefg",12);
   // to_encode contains the string to be encoded
   // (added a backslash at the correct spot)

   if (enc->encode(to_encode, out, true,true))
     printf ("This is the result: %s\n", (char *) out.get(1));

   // Displayed on screen:
   // This is the result: Test \a\b\c\defg

   return 0;
}
