redo-ifchange $OUT/wvtestmain
./wvtestrun redo "$OUT/$2.runtest"
redo-ifchange "$OUT/$2.runtest"
