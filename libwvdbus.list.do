exec >&2
redo-ifchange sources.list config.od $OUT/libwvbase.list
. ./config.od

if [ "$with_dbus" = "no" ]; then
    echo "No dbus; why are we building this?" >&2
    exit 99
else
    dirs="dbus"
    . ./objlist.od |
    comm -2 -3 - $OUT/libwvbase.list >$3
fi &&
redo-stamp <$3
