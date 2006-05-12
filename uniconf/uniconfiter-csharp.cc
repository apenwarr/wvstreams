#include "uniconfiter-csharp.h"
#include "uniconf.h"

uniconfiter_t uniconf_iter_init(uniconf_t _uni)
{
   UniConf *uni = (UniConf*)_uni; 
   UniConf::Iter *i = new UniConf::Iter(*uni);
   return (uniconfiter_t)i;
}

void uniconf_iter_free(uniconfiter_t iter)
{
    delete (UniConf::Iter*)iter;
}

void uniconf_iter_rewind(uniconfiter_t iter)
{
    UniConf::Iter *i = (UniConf::Iter*)iter;
    i->rewind();
}

int uniconf_iter_next(uniconfiter_t iter)
{
    UniConf::Iter *i = (UniConf::Iter*)iter;
    return i->next();
}

uniconf_t uniconf_iter_cur(uniconfiter_t iter)
{
    UniConf::Iter *i = (UniConf::Iter*)iter;
    return (uniconf_t)i->ptr();
}


