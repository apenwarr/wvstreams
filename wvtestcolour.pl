#!/usr/bin/perl
# 
# Usage: <run test> 2>&1 | wvtestcolour.pl
# 

while(<STDIN>)
{
    $_ =~ s/ok(\s+)$/\033[32mok\033[0m$1/;
    $_ =~ s/FAILED(\s+)/\033[1;31mFAILED\033[0m$1/;
    $_ =~ s/^(!\D+\d+)/\033[1;34m$1\033[0m/;
    $_ =~ s/^([^<]+<[^>]+>:)/\033[1;33m$1\033[0m/;
    
    print "$_";
}
