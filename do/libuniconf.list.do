redo-ifchange $OUT/sources.list $OUT/libwvbase.list

dirs="uniconf uniconf/daemon"

. do/objlist.od |
grep -Fv uniconf/daemon/uniconfd.o |
grep -Fv uniconf/unigenhack_s.o |
comm -2 -3 - $OUT/libwvbase.list >$3
redo-stamp <$3
