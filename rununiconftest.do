redo-ifchange $OUT/uni $OUT/uniconfd
cd $OUT
DAEMON=0 $SRCDIR/uniconf/tests/unitest.sh
DAEMON=1 $SRCDIR/uniconf/tests/unitest.sh
