redo-ifchange sources.list config.od $OUT/libwvbase.list
. ./config.od

[ "$with_dbus" = "no" ] && die "No dbus; why are we building this?"

dirs="dbus"
. ./objlist.od |
comm -2 -3 - $OUT/libwvbase.list >$3

redo-stamp <$3
