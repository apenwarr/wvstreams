#ifndef __UNICONF_CS_H
#define __UNICONF_CS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uniconfroot-csharp.h"
#include "uniconfkey-csharp.h"
//#include "iuniconfgen-csharp.h"

typedef void * uniconf_t;
//typedef void * uniconfroot_t;
//typedef void * uniconfkey_t;

uniconf_t uniconf_init();

uniconf_t uniconf_copy_init(uniconf_t uni);

void uniconf_free(uniconf_t);

uniconf_t uniconf_root(uniconf_t uni) ;

uniconf_t uniconf_parent(uniconf_t uni) ;

uniconfroot_t uniconf_rootobj(uniconf_t uni) ;

int uniconf_isnull(uniconf_t uni) ;

uniconfkey_t uniconf_fullkey(uniconf_t uni) ;

uniconfkey_t uniconf_key(uniconf_t uni) ;

const char * uniconf_key_str(uniconf_t uni);

const char * uniconf_fullkey_str(uniconf_t uni);

const uniconf_t uniconf_u(uniconf_t uni, const uniconfkey_t) ;

const uniconf_t uniconf_us(uniconf_t uni, const char *);

void uniconf_prefetch(uniconf_t uni, int) ;

const char *uniconf_getme(uniconf_t uni, const char *) ;

const char *uniconf_xget(uniconf_t uni, const char *, const char *) ;

int uniconf_getmeint(uniconf_t uni, int) ;

int uniconf_xgetint(uniconf_t uni, const char *, int) ;

int uniconf_exists(uniconf_t uni) ;

void uniconf_setme(uniconf_t uni, const char *) ;

void uniconf_xset(uniconf_t uni, const char *, const char *) ;

void uniconf_setmeint(uniconf_t uni, int) ;

void uniconf_xsetint(uniconf_t uni, const char *, int) ;

void uniconf_move(uniconf_t uni, const uniconf_t) ;

void uniconf_remove(uniconf_t uni) ;

void uniconf_copy(uniconf_t uni, const uniconf_t, int) ;

void uniconf_refresh(uniconf_t uni) ;

void uniconf_commit(uniconf_t uni) ;

/*
iuniconfgen_t *mount(const char *, bool) ;

iuniconfgen_t *mountgen(iuniconfgen_t, bool) ;

void ismountpoint() ;

bool isok() ;

iuniconfgen_t *whichmount(uniconfkey_t) ;
*/

#ifdef __cplusplus
}
#endif

#endif
