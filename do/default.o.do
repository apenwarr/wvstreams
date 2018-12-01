redo-ifchange $OUT/compile
dir=$(dirname "$2")
[ -d "$OUT/$dir" ] || mkdir -p "$OUT/$dir"
. $OUT/compile
