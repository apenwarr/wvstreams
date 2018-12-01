redo-ifchange $OUT/libs.list
[ -n "$DESTDIR" ] || die "set DESTDIR before installing."

$INSTALL -d "$DESTDIR$libdir"
for d in $(cat $OUT/libs.list); do
    [ "$d" = "libwvstatic" ] && continue
    redo-ifchange "$OUT/$d.so"
    $INSTALL_PROGRAM "$OUT/$d.so.$SO_VERSION" "$DESTDIR$libdir/"
done
$INSTALL -d "$DESTDIR$sysconfdir"
$INSTALL_DATA uniconf/daemon/uniconf.conf "$DESTDIR$sysconfdir/"
