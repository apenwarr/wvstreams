#include <wvstrutils.h>

int main(int argc, char *argv[])
{
    const char *input = "http://www.free_email-account.com/~ponyman/mail.pl?name=\'to|\\|Y |)4|\\|Z4\'&pass=$!J83*p&folder=1N8()><";

    for (int i=0; i<100000; i++)
    {
        if ((i % 100000) == 0)
        {
            printf("Done: %i\n", i);
        }
        url_encode(input);
    }

}
