#include "StdAfx.h"
#include ".\mysocket.h"

MySocket::MySocket(WvStream *_clone) :
    WvStreamClone(_clone)
{
    uses_continue_select = true;
}

void MySocket::execute()
{
    WvStreamClone::execute();

    print("Hello world! What is your name?\n");
    WvString name = getline(-1);
    print("Nice to meet you, %s. Any last words?\n", name);
    WvString lastwords = getline(-1);
    printf("Goodbye!\n");
    close();
}

MySocket::~MySocket(void)
{
}
