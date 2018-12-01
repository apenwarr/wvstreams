redo-ifchange $OUT/$2.list

obj=$(
    sed -e 's/unigenhack\.o/unigenhack_s\.o/' "$OUT/$2.list" |
    while read x; do
        echo "$OUT/$x"
    done
)
redo-ifchange $obj
$AR q $3 $obj
