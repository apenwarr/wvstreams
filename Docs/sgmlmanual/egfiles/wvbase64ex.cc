/*
 * A WvBase64 example.
 *
 */

#include "wvbase64.h"
#include "wvstream.h"
#include "wvstreamlist.h"
#include "wvencoderstream.h"
#include "wvbufbase.h"

int main()
{
   WvEncoder *enc;
   enc = new WvBase64Encoder();

   WvInPlaceBuf to_encode(100);
   WvInPlaceBuf encoded(100);

   to_encode.put("123",3);
   // to_encode contains the string to be encoded in base64

   if (enc->encode(to_encode, encoded, true,true))
     printf ("This is the result: %s\n", (char *) encoded.get(1));

   // Displayed on screen:
   // This is the result: MTIz


   WvEncoder *dec;
   dec = new WvBase64Decoder();

   WvInPlaceBuf to_decode(100);
   WvInPlaceBuf decoded(100);

   to_decode.put("MTIz",4);
   // to_encode contains the string to be encoded in base64

   if (dec->encode(to_decode, decoded, true))
     printf ("This is the result: %s\n", (char *) decoded.get(1));

   // Displayed on screen:
   // This is the result: 123

   return 0;
}
