#ifndef __WIN32_SANITIZE_H
#define __WIN32_SANITIZE_H

#include "wvautoconf.h"
#include <basetyps.h>
#include <objbase.h>
#include <signal.h>

#define _SYS_GUID_OPERATOR_EQ_ 1

#ifndef SIGALRM
#define SIGALRM 14
#endif

#ifndef SIGPIPE
#define SIGPIPE 13
#endif

// FIXME: this makes alarms silently fail.  They should probably fail more
// nicely, or (better still) actually work...
static inline unsigned int alarm(unsigned int t) { return 0; }

#endif // __WIN32_SANITIZE_H
