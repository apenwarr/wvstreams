case $2 in
  uniconf/tests/uni|uniconf/daemon/uniconfd)
    [ -e "$2.cc" -o -e "$2.c" ] || exit 1
    objs="$2.o libuniconf.so libwvstreams.so libwvutils.so libwvbase.so"
    libs="$LIBS_PAM $LIBS_SSL $LIBS"
    . ./link.od
    ;;
  */tests/*)
    [ -e "$2.cc" -o -e "$2.c" ] || exit 1
    objs="$2.o libwvstatic.so"
    libs="$LIBS_QT $LIBS_PAM $LIBS_DBUS $LIBS_SSL $LIBS"
    . ./link.od
    ;;
  *)
    echo "Error: $1: no rule to build this target." >&2
    exit 1
    ;;
esac

