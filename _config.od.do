exec >&2
redo-ifchange $OUT/config.mk &&
sed -e 's/=\(.*\)/="\1"/' <$OUT/config.mk >$3
redo-stamp <$3
