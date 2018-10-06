redo-ifchange config.od
. ./config.od

if [ -z "$USE_WVSTREAMS_ARGP" ]; then
    echo "Error: don't need argp; don't build libargp.list!" >&2
    exit 99
fi

make -C "$OUT/argp" &&
(
    cd "$OUT" &&
    for d in argp/*.o; do
        [ "$d" = "argp/argp-test.o" ] && continue
        echo "$d"
    done
) >$3
