#!/bin/bash
# Test various UniConf stuff using the 'uni' program.
#
# Usage: unitest.sh [section/number]
#   Where 'section/number' is the test section name or test number to run.
#   If not given, we run all tests from all sections.
#
export UNICONF
PATH=".:$PATH"

RUNWHICH="$1"

# do you want to test the uniconfd, or just straight ini files?
[ -z "$DAEMON" ] && DAEMON=0

# don't play with these
TESTNUM=1
FAILS=0
SECT=""
IFS="
"

RECONFIG()
{
    killall -q uniconfd
    killall -q -9 uniconfd
    rm -f $PWD/unisocket
    
    XDAEMON="$DAEMON"
    if [ "$1" = "--daemon" ]; then
        XDAEMON=1
	shift
    fi
    
    if [ "$XDAEMON" != "1" ]; then
        UNICONF="$@"
    else
        echo "Starting UniConfDaemon."
        
        # FIXME: we should be able to avoid the 'sleep' here, but
        # WvDaemon is currently buggy in that it forks into the background
        # *before* it finishes running the daemon setup code.  Ew.
        #
        # Furthermore, we can't actually use WvDaemon's fork-to-background
        # because it *also* does chdir("/") as part of that, which happens
        # before opening the ini file, which is expected to be in the current
        # directory.  Anyway, same bug: chdir and fork *after* setup.
	../daemon/uniconfd -lunix:$PWD/unisocket "$@" &
	sleep 1
	UNICONF="unix:$PWD/unisocket"
    fi
}

TESTMATCH()
{
    if [ -z "$RUNWHICH" \
         -o "$SECT" = "$RUNWHICH" \
	 -o "$TESTNUM" = "$RUNWHICH" ]; then
        return 0  # true
    else
        return 1  # false
    fi
}

SECTION()
{
    SECT="$1"
    shift
    TESTMATCH || return
    
    echo
    echo "Testing \"$@ ('$SECT')\" in unitest.sh:"
}

_check()
{
    teststr=$(echo -n "$@" | perl -e '@a=<>; $_=join("",@a); 
                     s/[\n\t]/!!/mg; printf("%-30.30s", $_);')
    echo -n "! unitest.sh:$SECT:$TESTNUM $teststr   "
    if [ "$@" ]; then
        echo "ok"
    else
        echo "FAILED"
	# echo -n "Failure: ($@)" | perl -pe 's/\n/!/mg;'
	echo
	FAILS=$(($FAILS + 1))
    fi
}

check()
{
    echo "ONE='$ONE'"
    echo "TWO='$TWO'"
    TESTMATCH && _check "$@"
    TESTNUM=$(($TESTNUM + 1))
}

x()
{
    echo " x: '$*'"
    TESTMATCH && ONE=$("$@" || { [ $? -gt 127 ] && echo "UNI DIED!"; })
}

xx()
{
    echo "xx: '$*'"
    TESTMATCH && TWO=$("$@" || { [ $? -gt 127 ] && echo "UNI DIED!"; })
    check "$(echo "$ONE" | sort)" = "$(echo "$TWO" | sort)"
}

s()
{
    TESTMATCH && ONE="$*"
}

ss()
{
    TESTMATCH && TWO="$*"
    check "$(echo "$ONE" | sort)" = "$(echo "$TWO" | sort)"
}



SECTION tester "Testing the tester"
check 1 = 1
check "1  1" = $(echo "1  1")
 s "1  2"
ss "1  2"
xx echo "1  2"
 x echo 1 "" 1
ss 1\ \ 1
 s 1 2 3
ss 2 1 3

SECTION null "null generator tests"
RECONFIG null:
 s
xx uni get /a
xx uni get /a/b
RECONFIG --daemon null:  # needs persistence
xx uni set /a/b foo
xx uni get /a/b

SECTION temp "temp generator tests"
RECONFIG temp:
 s
xx uni get /a
xx uni get /a/b
RECONFIG --daemon temp:   # needs persistence
xx uni set /a/b foo
 s foo
xx uni get /a/b

SECTION ini "ini file tests"
RECONFIG ini:simple.ini
 x uni get /section1/a
ss 1
 x uni get /section1/a/b
ss
 x uni keys /
ss section0 section1 section2 section3 \*
 x uni hkeys section2
ss a2 b2 c2 d2 e2
 x uni dump section2
ss "a2 = 11" "b2 = 22" "c2 = 33" "d2 = 44" "e2 = 55"


# FIXME: this test fails with the daemon, passes without!
# It's related to the trailing slash.
#
#RECONFIG ini:simple2.ini
# x uni hdump /section3/
#ss "a3 = {}" "* = default star" "a3/bog = alternative bog"


SECTION list "list (try-alternatives) generator tests"
RECONFIG "list: ini:simple.ini"
 s 1
xx uni get /section1/a
 s section0 section1 section2 section3 \*
xx uni keys /
RECONFIG "list: ini:simple2.ini ini:simple.ini"
 s 1
xx uni get /section1/a
xx uni get /section1/xa
 s section0 section1 section2 section3 section4 \*
xx uni keys /
 s a b c d e xa xb xc xd xe
xx uni keys /section1
 s a2 b2 c2 d2 e2
xx uni keys /section2
 s "a2 = 1x" "b2 = 2x" "c2 = 3x" "d2 = 44" "e2 = 55"
xx uni dump /section2



# FIXME: MOST OF THE FOLLOWING TESTS FAIL!!!

if false; then


SECTION default "Default (*/*) generator tests"
RECONFIG "default: null:"
 s
xx uni get /anything
RECONFIG default:ini:simple.ini
 s 1
xx uni get /section1/a
 s "the standard bog"
xx uni get /section1/a/bog
 s
xx uni get /frog/snicker/bog # FIXME: no frog/snicker; bog shouldn't exist
 s a a/bog b b/bog c c/bog d d/bog e e/bog
xx uni hkeys /section1  # FIXME: iter should list 'bog', but currently doesn't
 s
xx uni get '/*/*/bog' # FIXME: stars should be hidden by the default: filter
xx uni hkeys '/*'     # FIXME: stars should be hidden by the default: filter

# the list: directive here should make no difference at all.
SECTION deflist "Default/list combination tests"
base="ini:simple.ini"
for uri in "default:$base" "list:default:$base" "default:list:$base"; do
    RECONFIG "$uri"
     s 111
    xx uni get /section3/a3
     s "the standard bog"  
    xx uni get /section3/a3/bog # FIXME: this one fails ONLY with second uri!!
     s a a/bog b b/bog c c/bog d d/bog e e/bog
    xx uni keys section1 # FIXME: this one fails because plain default: fails
done
RECONFIG "default:list: ini:simple2.ini ini:simple.ini"
 s 1x
xx uni get /section2/a2
 s 1
xx uni get /section1/a
 s "default star"
xx uni get /section3/anything
 s "random subkey"
xx uni get /section2/anything


SECTION wvconf "WvConf wrapper tests"
RECONFIG wvconf:simple.ini
 s 5
xx uni get /section1/e
 s section0 section1 section2 section3 \*
xx uni keys /  # FIXME: iterating at the top level doesn't work
 s a b c d e
xx uni keys /section1
 s bog
xx uni keys '*/*'
 s "the standard bog"
xx uni get '/*/*/bog'  # FIXME: 'keys' lists the key, but 'get' is blank!
 s \* \*/bog
xx uni hkeys \*
FILE=__junk.ini
rm -f "$FILE"
touch "$FILE"
RECONFIG "wvconf:$FILE"
 s
xx uni set /section/entry "string"
 s "string"
xx uni get /section/entry  # FIXME: this fails if DAEMON=0
rm -f "$FILE"

fi  # disable failing tests

# FIXME: the cache needs much more thorough testing than these simple tests;
#   especially consider what happens on 'set' and if someone else writes to
#   the daemon you're using.
SECTION cache "Caching tests"
   # FIXME: all of these fail, implying that the cache massively sucks.
RECONFIG "cache:ini:simple2.ini"
 s "alternative bog"
xx uni get /section3/a3/bog
xx uni get /section3/a3/bog
 s section1 section2 section3 section4 \*
xx uni keys /
 s \* a3 a3/bog
xx uni hkeys /section3
RECONFIG "cache:list: ini:simple2.ini ini:simple.ini"
 s 1
xx uni get /section1/a
xx uni get /section1/xa
 s section0 section1 section2 section3 section4 \*
xx uni keys /
 s a b c d e xa xb xc xd xe
xx uni keys /section1
 s a2 b2 c2 d2 e2
xx uni keys /section2
 s "a2 = 1x" "b2 = 2x" "c2 = 3x" "d2 = 44" "e2 = 55"
xx uni dump /section2
RECONFIG --daemon "ini:simple2.ini"  # daemon itself is non-cached
UNICONF="cache:$UNICONF"  # but cache the connection to the daemon
 s "alternative bog"
xx uni get /section3/a3/bog
xx uni get /section3/a3/bog
 s section1 section2 section3 section4 \*
xx uni keys /
 s \* a3 a3/bog
xx uni hkeys /section3




DAEMON=
RECONFIG "null:"

echo
if [ "$FAILS" = 0 ]; then
    echo "All uniconf tests passed."
    exit 0
else
    echo "$FAILS uniconf tests failed!"
    exit 1
fi
