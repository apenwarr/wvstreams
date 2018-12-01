redo-ifchange $OUT/tests.list $OUT/wvtestmain

while read d; do
  echo "$OUT/$d"
done <$OUT/tests.list | xargs redo-ifchange
