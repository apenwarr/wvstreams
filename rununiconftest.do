exec >&2
redo-ifchange $OUT/uni $OUT/uniconfd
cd $OUT || exit 99
DAEMON=0 $SRCDIR/uniconf/tests/unitest.sh
DAEMON=1 $SRCDIR/uniconf/tests/unitest.sh
