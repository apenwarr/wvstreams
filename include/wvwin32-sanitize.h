#ifndef __WIN32_SANITIZE_H
#define __WIN32_SANITIZE_H

#ifdef _WIN32
#ifdef __GNUC__
#include "wvautoconf.h"
#endif

#include <basetyps.h>
#include <objbase.h>
#include <signal.h>
#include <winsock.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>

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

// FIXME: this makes alarms silently fail.  They should probably fail more
// nicely, or (better still) actually work...
static inline unsigned int alarm(unsigned int t) { return 0; }

// refer to _wvinitialize to ensure that we suck in some stuff that makes
// wvstreams actually work properly.
#ifdef __cplusplus
extern void *_wvinitialize;
static void *_wvinitialize_local = _wvinitialize;
#endif

// #define _alloca(x) alloca(x)

#endif // _WIN32

#endif // __WIN32_SANITIZE_H
