/* FIXME: horribly incomplete */
#include "wvtest.h"
#include "wvrsa.h"

WVTEST_MAIN()
{
    // this test tests to make sure that the cause
    // of BUGZID:3850 hasn't re-appeared...
    // (pub and prv weren't being initialized when
    //  we were initializing with a NULL value)
    {
	WvRSAKey rsa(NULL, false);
	WVPASS(rsa.public_str() == WvString::null);
	WVPASS(rsa.private_str() == WvString::null);
	WVFAIL(rsa.isok());
    }
    {
	WvRSAKey rsa(NULL, true);
	WVPASS(rsa.private_str() == WvString::null);
	WVPASS(rsa.public_str() == WvString::null);
	WVFAIL(rsa.isok());
    }
}

static const char rsa_private[] = "3082025b02010002818100e16d0efd5b89d50761fc6"
    "b4befad25cdddfdcfc9409013e01984bb6ea88f45404df25f0f7e89d39295df774c0fca14"
    "91e0bb731e4af179ad3f3585671ac78002bdca7de4007ad575360d4335d3eb5f261124003"
    "62ee33301a947b725ce0a65bb5c4de0b66eb501032b90fd35d0faac9b7d1c1bf5dcb4fb7a"
    "9f128c59ed9073f50203010001028180325dfeb2672885bb8f8e299f1edf2e0a30668c6da"
    "80a4916923d10efe9a391528bd7f29b70a774e954a9486b6b3fb896db82a6770741aaf125"
    "a55cb82bc895761bd5747732c45dc1e7d58ee89fe4f0d359270e5d942ab9ae41c161b2ab5"
    "3ba990e64b8c93791a859ab08a026a48f7446d9aaca75279db7e8b245a9c82b6987250241"
    "00fdaaa281dfee19c0e959c1bcb47f3acf9243377e5884244828773e3fa2c973b4b9d4167"
    "43fd41f4d23471ea2e3d5354bf0807eccf1695756dd477ff8fe7c8467024100e37feb62d7"
    "01e4bb4849950cf5db2515a70764f7c7ae3461fe8e807c586c13a1ac01fa446a2a2740744"
    "1c58d297ec0daf9a77292a2d4e816807d2073a78fab43024066b5b8a72dac92f0f18b4e4e"
    "c226e2013a0fcd607326ce2a09787ed3f56dec53b90a8f2cf2cb49014acf79302b6020fc6"
    "69d20ba8ae5445fffa8fbc02e0aecf102406097c5a797c6b40958adf55d255e40a6aade96"
    "de25a82f9193f58954426ed0ff09fb64f97b621e7c5d6037b2b1f5a188d80b62b823eee60"
    "3f7d628db323febe502403f03ed55da41c33011c61014f7c6bd0d21be0292f586802f9e9c"
    "7e141016f2ac58b958980d874f2cc9d902b2be25ee900900cffe729d5401f1ad784992a1b"
    "3db";

static const char rsa_public[] = "30818902818100e16d0efd5b89d50761fc6b4befad25"
    "cdddfdcfc9409013e01984bb6ea88f45404df25f0f7e89d39295df774c0fca1491e0bb731"
    "e4af179ad3f3585671ac78002bdca7de4007ad575360d4335d3eb5f26112400362ee33301"
    "a947b725ce0a65bb5c4de0b66eb501032b90fd35d0faac9b7d1c1bf5dcb4fb7a9f128c59e"
    "d9073f50203010001";

// like rsa_public, but with a small bit of corruption
static const char rsa_junk[] = "30818902818100e16d0efd5b89d50761fc6b4befad25"
    "cdddfdcfc9409013e01984bb6ea88f45404df25f0f7e89d39295df774c0fca1491e0bb731"
    "e4af179ad3f3585671ad78002bdca7de4007ad575360d4335d3eb5f26112400362ee33301"
    "a947b725ce0a65bb5c4de0b66eb501032b90fd35d0faac9b7d1c1bf5dcb4fb7a9f128c59e"
    "d9073f50203010001";

// like rsa_public, but truncated
static const char rsa_junk2[] = "30818902818100e16d0efd5b89d50761fc6b4befad25"
    "cdddfdcfc9409013e01984bb6ea88f45404df25f0f7e89d39295df774c0fca1491e0bb731"
    "e4af179ad3f3585671ac78002bdca7de4007ad575360d4335d3eb5f26112400362ee33301"
    "a947b725ce0a65bb5c4de0b66eb501032b90fd35d0faac9b7d1c1bf5dcb4fb7a9f128c59e"
    "d9073f502030100";

// like rsa_private, but with a small bit of corruption
static const char rsa_junk3[] = "3082025b02010002818100e16d0efd5b89d50761fc6"
    "b4befad25cdddfdcfc9409013e01984bb6ea88f45404df25f0f7e89d39295df774c0fca14"
    "91e0bb731e4af179ad3f3585671ac78002bdca7de4007ad575360d4335d3eb5f261124003"
    "62ee33301a947b725ce0a65bb5c4de0b66eb501032b90fd35d0faac9b7d1c1bf5dcb4fb7a"
    "9f128c59ed9073f50203010001028180325dfeb2672885bb8f8e299f1edf2e0a30668c6da"
    "80a4916923d10efe9a391528bd7f29b70a774e954a9486b6b3fb896db82a6770741aaf125"
    "a55cb82bc895761bd5747732c45dc1e7d58ee89fe4f0d359270e5d942ab9ae41c161b2ab5"
    "3ba990e64b8c93791a859ab08a026a48f7446d9aaca75279db7e8b245a9c82b6987250241"
    "00fdaaa281dfee19c0e959c1bcb47f3acf9243377e5884244828773e3fa2c973b4b9d4167"
    "43fd41f4d23471ea2e3d5354bf0807eccf1695756dd477ff8fe7c8467024100e37feb62d7"
    "01e4bb4849950cf5db2515a70764f7c7ae3461fe8e807c586c13a1ac01fa446a2a2740744"
    "1c58d297ec0daf9a77292a1d4e816807d2073a78fab43024066b5b8a72dac92f0f18b4e4e"
    "c226e2013a0fcd607326ce2a09787ed3f56dec53b90a8f2cf2cb49014acf79302b6020fc6"
    "69d20ba8ae5445fffa8fbc02e0aecf102406097c5a797c6b40958adf55d255e40a6aade96"
    "de25a82f9193f58954426ed0ff09fb64f97b621e7c5d6037b2b1f5a188d80b62b823eee60"
    "3f7d628db323febe502403f03ed55da41c33011c61014f7c6bd0d21be0292f586802f9e9c"
    "7e141016f2ac58b958980d874f2cc9d902b2be25ee900900cffe729d5401f1ad784992a1b"
    "3db";

WVTEST_MAIN()
{
    {
	// make sure isok() doesn't crash when a private key is given
	WvRSAKey rsa(rsa_private, true);
	printf("Error is: '%s'\n", rsa.errstr().cstr());
	WVPASS(rsa.isok());
    }
    
    {
	// make sure isok() doesn't crash when only a public key is given
	WvRSAKey rsa(rsa_public, false);
	printf("Error is: '%s'\n", rsa.errstr().cstr());
	WVPASS(rsa.isok());
    }

#if 0 // openssl can't verify public keys... sigh.
    {
	// catch invalid keys
	WvRSAKey rsa(rsa_junk, false);
	printf("Error is: '%s'\n", rsa.errstr().cstr());
	WVFAIL(rsa.isok());
    }
    
    {
	// catch invalid keys
	WvRSAKey rsa(rsa_junk2, false);
	printf("Error is: '%s'\n", rsa.errstr().cstr());
	WVFAIL(rsa.isok());
    }
#endif
    
    {
	// catch invalid keys
	WvRSAKey rsa(rsa_junk3, true);
	printf("Error is: '%s'\n", rsa.errstr().cstr());
	WVFAIL(rsa.isok());
    }
    
    {
	// make sure isok() doesn't crash when a public key is given instead
	// of a private key
	WvRSAKey rsa(rsa_public, true);
	printf("Error is: '%s'\n", rsa.errstr().cstr());
	WVFAIL(rsa.isok());
    }
}
