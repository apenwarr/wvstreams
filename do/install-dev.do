redo-ifchange $OUT/libs.list
[ -n "$DESTDIR" ] || die "set DESTDIR before installing."

$INSTALL -d \
    "$DESTDIR$libdir" \
    "$DESTDIR$includedir/wvstreams/xplc" \
    "$DESTDIR$bindir"
$INSTALL_DATA include/*.h "$DESTDIR$includedir/wvstreams"
$INSTALL_DATA include/xplc/*.h "$DESTDIR$includedir/wvstreams/xplc"
for d in $(cat $OUT/libs.list); do
    redo-ifchange "$OUT/$d.a"
    $INSTALL_PROGRAM "$OUT/$d.a" "$DESTDIR$libdir/"
    if [ "$d" != libwvstatic ]; then
        rm -f "$DESTDIR$libdir/$d.so"
        $LN_S "$d.so.$SO_VERSION" "$DESTDIR$libdir/$d.so"
    fi
done
for d in $OUT/pkgconfig/*.pc; do
    case $d in
      *-uninstalled.pc.in) ;;
      *.pc.in) $INSTALL_DATA $d "$DESTDIR$libdir/pkgconfig"
    esac
done
$INSTALL_SCRIPT wvtestrun "$DESTDIR$bindir"
$INSTALL -d "$DESTDIR$sysconfdir"
