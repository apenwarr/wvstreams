redo-ifchange $OUT/sources.list $OUT/libwvbase.list

dirs="
  streams configfile ipstreams urlget
  xplc xplc-cxx
"
[ -z "$_LINUX" ] || dirs="$dirs linuxstreams"
[ "$with_openssl" = "no" ] || dirs="$dirs crypto"

. do/objlist.od |
comm -2 -3 - $OUT/libwvbase.list >$3
redo-stamp <$3
