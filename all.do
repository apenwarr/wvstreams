redo-ifchange config.od
. ./config.od

dbus=
[ "$with_dbus" = "no" ] || dbus="$OUT/libwvdbus.so"
[ "$with_qt" = "no" ] || qt="$OUT/libwvqt.so"

redo-ifchange \
    $OUT/libuniconf.so \
    $OUT/libwvstreams.so \
    $OUT/libwvutils.so \
    $OUT/libwvbase.so \
    $OUT/libwvtest.so \
    $dbus $qt \
    $OUT/libwvstatic.so \
    $OUT/libwvstatic.a \
    $OUT/uni \
    $OUT/uniconfd

