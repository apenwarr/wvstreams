#include "uniconfconn.h"

const int UniConfConn::OPCODE_LENGTH = 4;

UniConfConn::UniConfConn(WvStream *s) : WvStreamClone(s)
{
    uses_continue_select = true;
}

UniConfConn::~UniConfConn()
{
    // end our continue_select here
    terminate_continue_select();
    close();
}

// Read the specified number of bytes into a character array, then set the
// queueminimum to be <nextsize>.  nextsize defaults to 0
bool UniConfConn::doread(char *buffer, long length, int nextsize)
{
    int bytes_read = 0;
    while (bytes_read == 0 && isok())
    {
        bytes_read = read(buffer, length);
        if (bytes_read != 0)
            break;
        continue_select(500);
    }
    buffer[length] = '\0';
    queuemin(nextsize);
    //if (bytes_read > 0)
        //wvcon->print("Read %s bytes to get:  %s\n", bytes_read, buffer);
    return isok();
}

// Read a long from the stream into the provided long, and set the
// queueminimum to be <nextsize>.  nextsize defaults to 0.
bool UniConfConn::doread(long *size, int nextsize)
{
    int bytes_read = 0;
    while (bytes_read == 0 && isok())
    {
        bytes_read = read(size, sizeof(*size));
        if (bytes_read != 0)
            break;
        continue_select(500);
    }
    *size = ntohl(*size);
    nextsize ? queuemin(nextsize) : queuemin(*size);
    //wvcon->print("Read %s bytes to get: %s\n", bytes_read, *size );
    return isok();
}

void UniConfConn::execute()
{
    WvStreamClone::execute();
}
