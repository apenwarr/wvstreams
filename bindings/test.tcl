
load ./libuniconf_tcl.so UniConf

set uni [uniconf_init temp:]

uniconf_set $uni foo bar
puts [uniconf_get $uni foo]

uniconf_set $uni foo baz
puts [uniconf_get $uni foo]
