redo-ifchange $OUT/uni $OUT/uniconfd config.od
. ./config.od

cd $OUT
export _WIN32 SRCDIR
if [ -n "$_WIN32" ]; then
    export WINE=wine64
fi
DAEMON=0 $SRCDIR/uniconf/tests/unitest.sh
if [ -z "$_WIN32" ]; then
    # Dealing with subprocesses in win32 is too annoying
    DAEMON=1 $SRCDIR/uniconf/tests/unitest.sh
fi
