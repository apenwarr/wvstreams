#!/bin/bash
#
# Post-process wvcrash.txt output using gdb, to try to figure out
# exactly which line things died on.
#
log()
{
	echo wvcrashread: "$@" >&2
}

agdb()
{
	(
		echo "set width 2000"
		echo "set height 20000"
		cat
		echo "quit"
	) | gdb --noasync -q --command=/dev/stdin "$1"
}


if [ -n "$1" ]; then
	log "Reading input from '$1'"

	if [ ! -r "$1" ]; then
		log "'$1' is not readable!  Aborting."
		exit 1
	fi

	exec <"$1"
fi

read binary junk
echo "$binary $junk"

log "wvcrash came from program: '$binary'"
if [ ! -x "$binary" -o -d "$binary" ]; then
	log "file '$binary' is not executable!  Aborting."
	exit 2
fi

read line
echo "$line"
read line
echo "$line"

if [ "$line" != "Backtrace:" ]; then
	log "Unrecognized wvcrash output (expected 'Backtrace:').  Aborting."
	exit 3
fi

SIMPLE=$(perl -ne '
	# parse the weird-looking output into four simple columns
	if ($_ =~ /^(.*)\((.*)\+(.*)\)\[(.*)\]/)
	{
		my ($bin, $func, $ofs, $abs) = ($1, $2, $3, $4);
		print "$bin $func $ofs $abs\n";
	}
	elsif ($_ =~ /^(.*)\[(.*)\]/)
	{
		my ($bin, $abs) = ($1, $2);
		print "$bin . 0x0 $abs\n";
	}
')

GDBSCRIPT=$(echo "$SIMPLE" | (
	while read bin func ofs abs junk; do
		echo "echo $bin $func $ofs\n"
		if [ "$func" = "." ]; then
			echo "echo ...\n"
		else
			echo "info addr $func"
		fi
	done
))

SIMPLE2=$(echo "$GDBSCRIPT" | agdb "$binary" |
perl -ne '
	# parse the interleaved column/gdb output
	my ($bin, $func, $ofs) = split;
	my $addr = "0";
	$_ = <>;
	if ($_ =~ /^Symbol "(.*)" is .* (0x[0-9A-Za-z]+)/)
	{
		$func = $1;
		$addr = $2;
	}
	print "$bin $func $addr+$ofs\n";
')

GDBSCRIPT2=$(echo "$SIMPLE2" | (
	while read bin func addr; do
		echo "echo $bin $func $addr\n"
		echo "info line *$addr"
	done
))

echo "$GDBSCRIPT2" | agdb "$binary" |
perl -ne '
	# parse the interleaved column/gdb output
	my ($bin, $func, $addr) = split;
	my $file = "--";
	my $line = "--";
	$_ = <>;
	
	$func = "file: $bin" if $func eq ".";
	
	if ($_ =~ /^Line ([0-9]*) of "(.*)" starts at /)
	{
		$file = $2;
		$line = $1;
	}
	
	$file =~ s,.*/,,g;
	
	printf "%-35s %s\n", $func, "$file:$line";
'
