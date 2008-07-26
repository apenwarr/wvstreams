#!/usr/bin/env perl
use warnings;
use strict;

my $a = $ARGV[0];

$\ = $/;
local $/;
while (<STDIN>)
{
    for (split(/(?<!\\)$/m))
    {
        s/^[^:]+:\s*/$a: /;
        print;
        if (s/^$a: //) {
            map { print "$_:" unless m/^\\$/ }
                 split(/\s+/);
        }
    }
}
