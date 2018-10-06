exec >&2
redo-ifchange sources.list config.od $OUT/libwvbase.list
. ./config.od

if [ "$with_qt" = "no" ]; then
    echo "No qt; why are we building this?" >&2
    exit 99
else
    dirs="qt"
    . ./objlist.od |
    comm -2 -3 - $OUT/libwvbase.list >$3
fi &&
redo-stamp <$3
