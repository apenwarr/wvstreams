#ifndef __UNICONFITER_CS_H
#define __UNICONFITER_CS_H

#include "uniconf-csharp.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void * uniconfiter_t;

uniconfiter_t uniconf_iter_init(uniconf_t uni);

void uniconf_iter_free(uniconfiter_t iter);

void uniconf_iter_rewind(uniconfiter_t iter);

int uniconf_iter_next(uniconfiter_t iter);

uniconf_t uniconf_iter_cur(uniconfiter_t iter);

#ifdef __cplusplus
}
#endif

#endif

