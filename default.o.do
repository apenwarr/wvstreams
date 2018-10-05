exec >&2
redo-ifchange $OUT/compile || exit
dir=$(dirname "$2")
[ -d "$OUT/$dir" ] || mkdir -p "$OUT/$dir"
. $OUT/compile
