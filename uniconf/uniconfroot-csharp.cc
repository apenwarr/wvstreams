#include "uniconfroot-csharp.h"
#include "uniconfroot.h"
#include "uniwatch.h"

static uniconfroot_cb callback;
static bool cc;

void uniconf_notify(const UniConf &uni, const UniConfKey &key)
{
    if (cc)
    {
        callback(key.printable());
    }
}
    
uniconfroot_t uniconfroot_init()
{
    UniConfRoot *root = new UniConfRoot();

    // Register a callback on all the keys
    root->add_callback(root, UniConfKey::EMPTY, uniconf_notify, true);
    cc = false;
    
    return (uniconfroot_t)root;
}

uniconfroot_t uniconfroot_moniker(const char *mon, int refresh)
{
   UniConfRoot *root = new UniConfRoot(mon, (bool)refresh);

    // Register a callback on all the keys
    root->add_callback(root, UniConfKey::EMPTY, uniconf_notify, true);
        
    cc = false;
    
    return (uniconfroot_t)root;
}

void uniconfroot_free(uniconfroot_t ur)
{
    UniConfRoot *root = (UniConfRoot*)ur;
    root->del_callback(root, UniConfKey::EMPTY, true);
    delete root;
}

void uniconfroot_setcb(uniconfroot_cb cb)
{
    callback = cb;
    cc = true;
}

