exec >&2
redo-ifchange config.od $OUT/$2.list
. ./config.od

obj=$(
    sed -e 's/unigenhack\.o/unigenhack_s\.o/' "$OUT/$2.list" |
    while read x; do
        echo "$OUT/$x"
    done
) &&
redo-ifchange $obj &&
$AR q $3 $obj
