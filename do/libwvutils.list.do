redo-ifchange $OUT/sources.list $OUT/libwvbase.list

argp=
if [ -n "$USE_WVSTREAMS_ARGP" ]; then
    redo-ifchange $OUT/libargp.list || exit
    argp=$(cat $OUT/libargp.list)
fi

dirs="utils"
[ -n "$_WIN32" ] && dirs="$dirs win32"

{
    . do/objlist.od &&
    [ -n "$argp" ] && echo "$argp"
} | sort |
comm -2 -3 - $OUT/libwvbase.list >$3

redo-stamp <$3
