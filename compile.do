redo-ifchange config.od $OUT/include/wvautoconf.h
. ./config.od

cat >$3 <<EOF

if [ -e "\$2.cc" ]; then
    cc="$CXX"
    cflags="$CPPFLAGS $CXXFLAGS"
    src="\$2.cc"
elif [ -e "\$2.c" ]; then
    cc="$CC"
    cflags="$CPPFLAGS $CFLAGS"
    src="\$2.c"
else
    echo "\$0: \$1: no source file found." >&2
    exit 1
fi
redo-ifchange "\$src" || exit

depfile="\$3.d"
\$cc -c "\$src" -o "\$3" \
    -Iinclude -I\$OUT/include \$cflags \
    -MMD -MF "\$depfile"
sed -e 1d -e 's/\\\\\$//' | xargs redo-ifchange
rm -f "\$depfile"

EOF
redo-stamp <$3
