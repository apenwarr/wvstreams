#ifndef UNICONFROOT_CS_H
#define UNICONFROOT_CS_H

#ifdef __cplusplus
extern "C" {
#endif
    
typedef void(*uniconfroot_cb)(const char *);

typedef void * uniconfroot_t;

uniconfroot_t uniconfroot_init();

uniconfroot_t uniconfroot_moniker(const char *, int);

void uniconfroot_free(uniconfroot_t ur);

void uniconfroot_setcb(uniconfroot_cb cb);

#ifdef __cplusplus
}
#endif

#endif
