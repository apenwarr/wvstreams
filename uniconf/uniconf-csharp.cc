#include "uniconf-csharp.h"
#include "uniconfiter-csharp.h"
#include "uniconfroot.h"
#include "uniconf.h"
#include "wvistreamlist.h"

void uniconf_rungloballist(int msec)
{
    WvIStreamList::globallist.runonce(msec);
}

uniconf_t uniconf_init()
{
    UniConf *uni = new UniConf();
    return (uniconf_t)uni;
}

void uniconf_free(uniconf_t uni)
{
    delete (UniConf*)uni;
}

uniconf_t uniconf_copy_init(uniconf_t uni)
{
    UniConf *r = new UniConf(*(UniConf*)uni);
    return (uniconf_t)r;
}

uniconf_t uniconf_root(uniconf_t uni) 
{
    UniConf r = ((UniConf*)uni)->root();
    return (uniconf_t)new UniConf(r);
}

uniconf_t uniconf_parent(uniconf_t uni) 
{
   UniConf r = ((UniConf*)uni)->parent();
   return (uniconf_t)new UniConf(r);
}

uniconfroot_t uniconf_rootobj(uniconf_t uni) 
{
    UniConfRoot *r = ((UniConf*)uni)->rootobj();
    return (uniconfroot_t)r;
}

int uniconf_isnull(uniconf_t uni) 
{
    bool r = ((UniConf*)uni)->isnull();
    return r;
}

uniconfkey_t uniconf_fullkey(uniconf_t uni) 
{
    UniConfKey r = ((UniConf*)uni)->fullkey();
    return (uniconfkey_t)new UniConfKey(r);
}

uniconfkey_t uniconf_key(uniconf_t uni) 
{
    UniConfKey r = ((UniConf*)uni)->key();
    return (uniconfkey_t)new UniConfKey(r);
}

const char * uniconf_key_str(uniconf_t uni)
{
    UniConfKey r = ((UniConf*)uni)->key();
    printf("DBG: %s\n", r.cstr());
    WvString *foo = new WvString(r.cstr());
    return foo->cstr();
}

const char * uniconf_fullkey_str(uniconf_t uni)
{
    UniConfKey r = ((UniConf*)uni)->fullkey();
    WvString *foo = new WvString(r.cstr());
    return foo->cstr();
}

const uniconf_t uniconf_u(uniconf_t uni, const uniconfkey_t k) 
{
    UniConfKey *kk = (UniConfKey*)k;
    UniConf r = ((UniConf*)uni)->u(*kk);
    return (uniconf_t)new UniConf(r);
}

const uniconf_t uniconf_us(uniconf_t uni, const char *str)
{
    UniConf r = ((UniConf*)uni)->u(str);
    return (uniconf_t)new UniConf(r);
}

void uniconf_prefetch(uniconf_t uni, int recurse) 
{
    ((UniConf*)uni)->prefetch((bool)recurse);
}

const char *uniconf_getme(uniconf_t uni, const char *def) 
{
    WvString s = ((UniConf*)uni)->getme(def);
    return s.cstr();
}

const char *uniconf_xget(uniconf_t uni, const char *k, const char *def) 
{
    WvString s = ((UniConf*)uni)->xget(k, def);
    return s.cstr();
}

int uniconf_getmeint(uniconf_t uni, int def) 
{
    return ((UniConf*)uni)->getmeint(def);
}

int uniconf_xgetint(uniconf_t uni, const char *k, int def) 
{
    return ((UniConf*)uni)->xgetint(k, def);
}

int uniconf_exists(uniconf_t uni) 
{
    return ((UniConf*)uni)->exists();
}

void uniconf_setme(uniconf_t uni, const char *v) 
{
    ((UniConf*)uni)->setme(v);
}

void uniconf_xset(uniconf_t uni, const char *k, const char *v) 
{
    ((UniConf*)uni)->xset(k, v);
}

void uniconf_setmeint(uniconf_t uni, int v) 
{
    ((UniConf*)uni)->setmeint(v);
}

void uniconf_xsetint(uniconf_t uni, const char *k, int v) 
{
    ((UniConf*)uni)->xsetint(k, v);
}

void uniconf_move(uniconf_t uni, const uniconf_t uu) 
{
    ((UniConf*)uni)->move(*((UniConf*)uu));
}

void uniconf_remove(uniconf_t uni) 
{
    ((UniConf*)uni)->remove();
}

void uniconf_copy(uniconf_t uni, const uniconf_t uu, int recurse) 
{
    UniConf &uuu = *(UniConf*)uu;
    ((UniConf*)uni)->copy(uuu, (bool)recurse);
}

void uniconf_refresh(uniconf_t uni) 
{
    ((UniConf*)uni)->refresh();
}

void uniconf_commit(uniconf_t uni) 
{
    ((UniConf*)uni)->commit();
}

/*
iuniconfgen_t *mount(const char *, bool) const;

iuniconfgen_t *mountgen(iuniconfgen_t, bool) const;

void ismountpoint() const;

bool isok() const;

iuniconfgen_t *whichmount(uniconfkey_t) const;
*/
