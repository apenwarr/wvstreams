exec >&2
redo-ifchange sources.list config.od $OUT/libwvbase.list
. ./config.od

dirs="uniconf uniconf/daemon"

. ./objlist.od |
grep -Fv uniconf/daemon/uniconfd.o |
grep -Fv uniconf/unigenhack_s.o |
comm -2 -3 - $OUT/libwvbase.list >$3
redo-stamp <$3
