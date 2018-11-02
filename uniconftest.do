exec >&2
redo-ifchange $OUT/uni $OUT/uniconfd
./wvtestrun redo "$OUT/rununiconftest"
