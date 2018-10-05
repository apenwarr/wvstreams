exec >&2
redo-always
find . -name '*.[ch]' -o -name '*.cc' | sort >$3
redo-stamp <$3
