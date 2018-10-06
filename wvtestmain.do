redo-ifchange $OUT/uniconfd $OUT/uni $OUT/$2.list || exit
objs=$(cat $OUT/$2.list)
. ./link.od
