exec >$3
echo libwvbase
echo libwvutils
echo libwvstreams
echo libuniconf
echo libwvstatic
[ "$with_dbus" = no ] || echo libwvdbus
[ "$with_qt" = no ] || echo libwvqt
redo-stamp <$3
