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
#include <sys/types.h>

#ifndef _SYS_GUID_OPERATOR_EQ_
#define _SYS_GUID_OPERATOR_EQ_ 1
#endif

#ifndef SIGALRM
#define SIGALRM 14
#endif

#ifndef SIGPIPE
#define SIGPIPE 13
#endif

#undef ECONNREFUSED
#define ECONNREFUSED WSAECONNREFUSED

#undef EWOULDBLOCK
#define EWOULDBLOCK WSAEWOULDBLOCK

typedef int socklen_t;

#ifdef __cplusplus
extern "C" {
#endif

unsigned int sleep(unsigned int secs);

#ifdef _WIN32
extern int getpid(void);
#else
extern pid_t getpid(void);
#endif

unsigned int alarm(unsigned int t);
int fsync(int fd);
    
#ifdef __cplusplus
}
#endif

#endif // __WIN32_SANITIZE_H
