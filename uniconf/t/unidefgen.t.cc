#include "wvtest.h"
#include "uniconfroot.h"
#include "unidefgen.h"

WVTEST_MAIN("defgen_single")
{
    UniConfRoot cfg("default:temp");

    cfg["/users/*"].setme("*1");
    cfg["/*/pooper"].setme("*1");
    cfg["/*/wonker"].setme("bonker");
    cfg["/*/foo"].setme("*3");
    cfg["/baz/foo"].setme("foobaz");

    WVPASSEQ(cfg["/snooper/pooper"].getme(), "snooper");
    WVPASSEQ(cfg["/bob/wonker"].getme(), "bonker");

    // FIXME: Is this the right behaviour? unidefgen.h says this is "undefined"
    WVPASSEQ(cfg["/bar/foo"].getme(), "");
    WVPASSEQ(cfg["/baz/foo"].getme(), "foobaz");

    cfg["/users/pooper"].setme("smarch");

    WVPASSEQ(cfg["/users/billybob"].getme(), "billybob");
    WVPASSEQ(cfg["/users/pooper"].getme(), "smarch");
}

WVTEST_MAIN("defgen_multi")
{
    UniConfRoot cfg("default:temp:");

    cfg["/*/silly/*/billy"].setme("*1");
    cfg["/willy/*/nilly/*"].setme("*2");
    cfg["/*/*/yoink"].setme("boink");

    cfg["/foo/silly/bar/billy"].setme("barfoo");
    cfg["/willy/bar/nilly/foo"].setme("foobar");

    WVPASSEQ(cfg["/cake/pie/yoink"].getme(), "boink");
    WVPASSEQ(cfg["/foo/silly/bar/billy"].getme(), "barfoo");
    WVPASSEQ(cfg["/bar/silly/foo/billy"].getme(), "foo");
    WVPASSEQ(cfg["/willy/bar/nilly/foo"].getme(), "foobar");
    WVPASSEQ(cfg["/willy/foo/nilly/bar"].getme(), "foo");
}

#if 0
//FIXME:I don't know whether this is supposed to work or not. (pcolijn)
//      It doesn't right now; if it's supposed to, uncomment and fix!
WVTEST_MAIN("defgen_phrases")
{
    UniConfRoot cfg("default:temp:");

    cfg["/*/foo/*/bar"].setme("*1 blah *2");
    cfg["/*/wonk/bonk/*"].setme("*2 blah *1");

    cfg["/zorp/foo/bork/bar"].setme("zorpbork");
    cfg["/plonk/wonk/bonk/honk"].setme("plonkhonk");

    WVPASSEQ(cfg["/baz/foo/zoo/bar"].getme(), "zoo blah baz");
    WVPASSEQ(cfg["/zorp/foo/bork/bar"].getme(), "zorpbork");
    WVPASSEQ(cfg["/turd/wonk/bonk/ferguson"].getme(), "turd blah ferguson");
    WVPASSEQ(cfg["/plonk/wonk/bonk/honk"].getme(), "plonkhonk");
}
#endif
