[ -n "$USE_WVSTREAMS_ARGP" ] || die "don't need argp; don't build libargp.list!"

make -C "$OUT/argp" >&2

(
    cd "$OUT" &&
    for d in argp/*.o; do
        [ "$d" = "argp/argp-test.o" ] && continue
        echo "$d"
    done
) >$3
