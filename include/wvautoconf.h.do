redo-ifchange ../config.od
. ../config.od

redo-ifchange $OUT/_wvautoconf.h
mkdir -p $OUT/include
cat $OUT/_wvautoconf.h >$3
redo-stamp <$3
