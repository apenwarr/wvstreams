redo-ifchange $OUT/$2.list

libs="$LIBS_QT $LIBS_PAM $LIBS_DBUS $LIBS_SSL $LIBS"

libs=
libdep=
case $2 in
  libwvbase) libs="$LIBS" ;;
  libwvutils) libdep="$OUT/libwvbase.so" libs="$LIBS_PAM $LIBS" ;;
  libwvstreams)
    libdep="$OUT/libwvutils.so $OUT/libwvbase.so"
    libs="$LIBS_SSL $LIBS"
    ;;
  libuniconf)
    libdep="$OUT/libwvstreams.so $OUT/libwvutils.so $OUT/libwvbase.so"
    libs="$LIBS_SSL $LIBS"
    ;;
  libwvdbus)
    libdep="$OUT/libwvstreams.so $OUT/libwvutils.so $OUT/libwvbase.so"
    libs="$LIBS_DBUS $LIBS"
    ;;
  libwvtest)
    libdep="$OUT/libwvbase.so"
    libs="$LIBS"
    ;;
  libwvstatic)
    libdep=
    libs="$LIBS_QT $LIBS_PAM $LIBS_DBUS $LIBS_SSL $LIBS"
    ;;
esac

obj=$(
    while read x; do
        echo "$OUT/$x"
    done <$OUT/$2.list
)

redo-ifchange $obj $libdep

sofile="$OUT/$2.so.$SO_VERSION"
zdefs=
[ -z "$_LINUX" ] || zdefs="-Wl,-z,defs"
rm -f "$sofile"
if [ -n "$_WIN32" ]; then
    echo "Skipping $1 on win32 (can't build shared libraries)" >&2
    redo-ifchange "$OUT/$2.a"
    ln -s $2.a "$sofile"
else
    $CXX -o "$sofile" -shared \
        $zdefs \
        $LDFLAGS \
        $obj $libdep $libs
fi
ln -s "$sofile" $3
