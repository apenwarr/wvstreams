#ifndef __WIN32_SANITIZE_H
#define __WIN32_SANITIZE_H

#ifdef __GNUC__
#include "wvautoconf.h"
#endif

#include <basetyps.h>
#include <objbase.h>
#include <signal.h>
#include <winsock.h>
#include <malloc.h>
#include <io.h>

#ifndef _SYS_GUID_OPERATOR_EQ_
#define _SYS_GUID_OPERATOR_EQ_ 1
#endif

#ifndef SIGALRM
#define SIGALRM 14
#endif

#ifndef SIGPIPE
#define SIGPIPE 13
#endif

#ifndef ECONNREFUSED
#define ECONNREFUSED WSAECONNREFUSED
#endif

#ifndef EWOULDBLOCK
#define EWOULDBLOCK WSAEWOULDBLOCK
#endif


typedef int socklen_t;

#ifdef __cplusplus
extern "C" {
#endif

unsigned int sleep(unsigned int secs);

typedef int pid_t;
extern pid_t getpid();

unsigned int alarm(unsigned int t);
int fsync(int fd);
    
#ifdef __cplusplus
}
#endif

#endif // __WIN32_SANITIZE_H
