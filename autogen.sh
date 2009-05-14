#!/bin/sh
set -e
autoheader
autoconf
(cd argp && autoconf)
