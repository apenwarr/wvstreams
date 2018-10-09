redo-ifchange config.od
. ./config.od

redo-ifchange $1.subst
sed -e 's/#VERSION#/$WVPACKAGE_VERSION/g' <$1.subst >$3
