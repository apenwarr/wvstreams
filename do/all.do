redo-ifchange $OUT/libs.list
{
    while read lib; do
      echo "$OUT/$lib.so"
    done <$OUT/libs.list
    echo $OUT/libwvstatic.a
    echo $OUT/uni
    echo $OUT/uniconfd
} | exec xargs redo-ifchange
