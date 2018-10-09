redo-ifchange config.od
. ./config.od

[ -n "$USE_WVSTREAMS_ARGP" ] || die "don't need argp; don't build libargp.list!"

make -C "$OUT/argp"

(
    cd "$OUT" &&
    for d in argp/*.o; do
        [ "$d" = "argp/argp-test.o" ] && continue
        echo "$d"
    done
) >$3
