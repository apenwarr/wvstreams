exec >&2
redo-ifchange $OUT/wvtestmain config.od
. ./config.od

if [ -n "$_WIN32" ]; then
    RUN=wine64
    VALGRIND=
else
    RUN=
fi

cd "$OUT"
$VALGRIND $RUN ./wvtestmain "$2"
