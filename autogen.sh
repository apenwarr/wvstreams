#!/bin/sh
set -e
autoheader config.ac
autoconf config.ac > configure.real
chmod 755 configure.real
(cd argp && autoconf)
