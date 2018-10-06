exec >&2
redo-ifchange sources.list config.od
. ./config.od

inp="
  $OUT/libwvbase.list
  $OUT/libwvutils.list
  $OUT/libwvstreams.list
  $OUT/libuniconf.list
"
  
[ "$with_dbus" = "no" ] || inp="$inp $OUT/libwvdbus.list"
[ "$with_qt" = "no" ] || inp="$inp $OUT/libwvqt.list"

redo-ifchange $inp &&
{
    cat $inp &&
    echo wvtest.o
} >$3 &&
redo-stamp <$3
