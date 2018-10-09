redo-ifchange sources.list config.od $OUT/libwvbase.list
. ./config.od

[ "$with_qt" = "no" ] && die "No qt; why are we building this?"

dirs="qt"
. ./objlist.od |
comm -2 -3 - $OUT/libwvbase.list >$3

redo-stamp <$3
