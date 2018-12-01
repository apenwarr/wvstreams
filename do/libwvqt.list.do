redo-ifchange $OUT/sources.list $OUT/libwvbase.list

[ "$with_qt" = "no" ] && die "No qt; why are we building this?"

dirs="qt"
. do/objlist.od |
comm -2 -3 - $OUT/libwvbase.list >$3

redo-stamp <$3
