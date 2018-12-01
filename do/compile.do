redo-ifchange $OUT/include/wvautoconf.h

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

\$cc -c "\$src" -o "\$3" \
    -Iinclude -I\$OUT/include \$cflags \
    -MMD -MF "\$3.d"
read DEPS <\$3.d
redo-ifchange \${DEPS#*:}
rm -f "\$3.d"

EOF
redo-stamp <$3
