#ifndef __WIN32_SANITIZE_H
#define __WIN32_SANITIZE_H

#include "wvautoconf.h"
#include <basetyps.h>
#include <objbase.h>
#include <signal.h>
#include <winsock.h>

#define _SYS_GUID_OPERATOR_EQ_ 1

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

// FIXME: this makes alarms silently fail.  They should probably fail more
// nicely, or (better still) actually work...
static inline unsigned int alarm(unsigned int t) { return 0; }

// refer to _wvinitialize to ensure that we suck in some stuff that makes
// wvstreams actually work properly.
#ifdef __cplusplus
extern void *_wvinitialize;
static void *_wvinitialize_local = _wvinitialize;
#endif

#endif // __WIN32_SANITIZE_H
