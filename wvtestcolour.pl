#!/usr/bin/perl
# 
# Usage: <run test> 2>&1 | wvtestcolour.pl
# 

while(<STDIN>)
{
    $_ =~ s/ok(\s+)$/\033[32mok\033[0m$1/;
    $_ =~ s/(FAIL|FAILED)(\s+)/\033[1;31m$1\033[0m$2/;
    $_ =~ s/^(!\s+\S+)/\033[1;34m$1\033[0m/;
    $_ =~ s/^([^<]+<[^>]+>:)/\033[1;33m$1\033[0m/;
    
    print "$_";
}
