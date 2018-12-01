exec >&2
redo-ifchange $OUT/sources.list not-win32-tests.list

dirs="
    utils/tests
    streams/tests
    ipstreams/tests
    configfile/tests
    linuxstreams/tests
    uniconf/tests
    crypto/tests
"
[ "$with_dbus" = "no" ] || dirs="$dirs dbus/tests"
[ "$with_qt" = "no" ] || dirs="$dirs qt/tests"

. do/objlist.od |
while read d; do
    echo "${d%.o}"
done | sort | {
    if [ -n "$_WIN32" ]; then
        comm -2 -3 - not-win32-tests.list
    else
        cat
    fi
} >$3
