redo-ifchange $OUT/sources.list $OUT/libwvbase.list

[ "$with_dbus" = "no" ] && die "No dbus; why are we building this?"

dirs="dbus"
. do/objlist.od |
comm -2 -3 - $OUT/libwvbase.list >$3

redo-stamp <$3
