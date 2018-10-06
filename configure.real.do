exec >&2
redo-ifchange config.ac &&
autoheader config.ac &&
autoconf config.ac >$3 &&
chmod 755 $3 &&
(cd argp && autoconf)
