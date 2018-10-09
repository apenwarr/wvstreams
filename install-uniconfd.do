redo-ifchange config.od $OUT/libs.list \
	$OUT/uni $OUT/uniconf/tests/uni.8 \
	$OUT/uniconfd $OUT/uniconf/daemon/uniconfd.8
. ./config.od
[ -n "$DESTDIR" ] || die "set DESTDIR before installing."

$INSTALL -d \
    "$DESTDIR$libdir" \
    "$DESTDIR$bindir" \
    "$DESTDIR$sbindir" \
    "$DESTDIR$localstatedir/lib/uniconf" \
    "$DESTDIR/$mandir/man8"
$INSTALL_PROGRAM "$OUT/uni" "$DESTDIR$bindir"
$INSTALL_PROGRAM "$OUT/uniconfd" "$DESTDIR$sbindir"
touch "$DESTDIR$localstatedir/lib/uniconf/uniconfd.ini"
$INSTALL_DATA "$OUT/uniconf/daemon/uniconfd.8" "$DESTDIR$mandir/man8"
$INSTALL_DATA "$OUT/uniconf/tests/uni.8" "$DESTDIR$mandir/man8"
