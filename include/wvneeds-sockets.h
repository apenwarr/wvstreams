#ifndef __WVNEEDS_SOCKETS_H
#define __WVNEEDS_SOCKETS_H

#ifdef _WIN32

// refer to _wvinitialize to ensure that we suck in some stuff that makes
// wvstreams actually work properly.
#ifdef __cplusplus
extern void *_wvinitialize;
static void *_wvinitialize_local = _wvinitialize;
#endif

#endif // _WIN32

#endif // __WVNEEDS_SOCKETS_H
