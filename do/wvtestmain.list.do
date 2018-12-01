redo-ifchange $OUT/sources.list

dirs=
dirs="$dirs
  utils
  streams configfile ipstreams urlget uniconf
  xplc xplc-cxx
"
[ -z "$_LINUX" ] || dirs="$dirs linuxstreams"
[ "$with_openssl" = "no" ] || dirs="$dirs crypto"
[ "$with_dbus" = "no" ] || dirs="$dirs dbus"
[ "$with_qt" = "no" ] || dirs="$dirs qt"
[ -n "$_WIN32" ] && dirs="$dirs win32"

for dir in $dirs; do
    for d in $dir/t/*.cc; do
        [ -e "$d" ] || continue
        echo "${d%.cc}.o"
    done
done |
sort |
{
    if [ -n "$_WIN32" ]; then
    	redo-ifchange not-win32-tests.list &&
        comm -2 -3 - not-win32-tests.list
    else
        cat
    fi
    echo wvtestmain.o
} >$3
redo-stamp <$3
